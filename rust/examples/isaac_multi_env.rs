//! Isaac Lab Backend Example
//!
//! Demonstrates GPU-accelerated physics simulation with parallel environments.
//!
//! Run with: cargo run --example isaac_multi_env --features isaac

use anyhow::Result;
use autonomysim_core::{
    backend::{BackendConfig, SimulationBackend},
    vehicle::{SensorSpec, SensorType, VehicleControl, VehicleSpec, VehicleType},
    Transform,
};
use nalgebra::{Point3, UnitQuaternion};

#[cfg(feature = "isaac")]
use autonomysim_backends::IsaacLabBackend;

#[tokio::main]
async fn main() -> Result<()> {
    println!("=== Isaac Lab Multi-Environment Simulation ===\n");

    #[cfg(not(feature = "isaac"))]
    {
        println!("❌ This example requires the 'isaac' feature");
        println!("   Run with: cargo run --example isaac_multi_env --features isaac");
        return Ok(());
    }

    #[cfg(feature = "isaac")]
    {
        // Initialize Isaac Lab with 4 parallel environments
        println!("Initializing Isaac Lab backend with 4 parallel environments...");

        let mut backend =
            IsaacLabBackend::with_config(autonomysim_backends::isaac::IsaacLabConfig {
                num_envs: 4,
                device: "cuda:0".to_string(),
                physics_dt: 0.01,
                enable_gpu_pipeline: true,
                enable_scene_graph: true,
                python_path: None,
            });

        backend.initialize(BackendConfig::default()).await?;
        println!("✓ Backend initialized\n");

        // Load scene
        println!("Loading scene...");
        let _scene = backend.load_scene("warehouse").await?;
        println!("✓ Scene loaded\n");

        // Spawn 8 drones (2 per environment)
        println!("Spawning 8 drones across 4 environments...");
        let mut vehicle_ids = Vec::new();

        for i in 0..8 {
            let env_id = i / 2; // 2 drones per environment
            let pos_offset = if i % 2 == 0 { 0.0 } else { 5.0 };

            let spec = VehicleSpec {
                vehicle_id: format!("drone_{}", i),
                vehicle_type: VehicleType::Multirotor,
                initial_transform: Transform::new(
                    Point3::new(pos_offset, 0.0, 10.0),
                    UnitQuaternion::identity(),
                ),
                parameters: Default::default(),
                sensors: vec![
                    SensorSpec {
                        sensor_id: "imu".to_string(),
                        sensor_type: SensorType::Imu,
                        update_rate_hz: 100.0,
                        enabled: true,
                    },
                    SensorSpec {
                        sensor_id: "gps".to_string(),
                        sensor_type: SensorType::Gps,
                        update_rate_hz: 10.0,
                        enabled: true,
                    },
                ],
            };

            let vehicle_id = backend.spawn_vehicle(spec).await?;
            vehicle_ids.push(vehicle_id);
            println!(
                "  ✓ Spawned {} in environment {}",
                format!("drone_{}", i),
                env_id
            );
        }

        println!("\n✓ {} vehicles spawned\n", vehicle_ids.len());

        // Run parallel simulation
        println!("Running parallel simulation (GPU-accelerated)...");
        println!(
            "All {} environments stepping in parallel on GPU\n",
            backend.config().num_envs
        );

        for step in 0..10 {
            // Step all environments simultaneously on GPU
            backend.step(0.1).await?;

            // Set control for all vehicles
            for vehicle_id in &vehicle_ids {
                let control = if step < 5 {
                    VehicleControl::hover()
                } else {
                    VehicleControl::forward(0.3)
                };
                backend.set_vehicle_control(vehicle_id, control)?;
            }

            // Print state for first vehicle in each environment
            if step % 3 == 0 {
                println!("Step {}: t = {:.1}s", step, backend.get_time());
                for env in 0..4 {
                    let vehicle_id = format!("drone_{}", env * 2);
                    let state = backend.get_vehicle_state(&vehicle_id)?;
                    println!(
                        "  Env {}: {} at ({:.2}, {:.2}, {:.2})",
                        env,
                        vehicle_id,
                        state.transform.position.x,
                        state.transform.position.y,
                        state.transform.position.z
                    );
                }
                println!();
            }
        }

        println!("\n=== GPU Raycasting Demo ===\n");
        println!("Isaac Lab uses GPU-accelerated raycasting (orders of magnitude faster than CPU)");

        // Cast rays from each drone
        let scene = backend.load_scene("warehouse").await?;
        for vehicle_id in vehicle_ids.iter().take(4) {
            let state = backend.get_vehicle_state(vehicle_id)?;
            let ray = autonomysim_core::Ray {
                origin: state.transform.position,
                direction: nalgebra::Vector3::new(0.0, 0.0, -1.0),
                max_distance: 100.0,
            };

            if let Some(hit) = backend.cast_ray(&scene, &ray)? {
                println!("  {}: Ground at {:.2}m below", vehicle_id, hit.distance);
            }
        }

        println!("\n=== Cleanup ===\n");
        println!("Removing vehicles...");
        for vehicle_id in vehicle_ids {
            backend.remove_vehicle(&vehicle_id).await?;
            println!("  ✓ Removed {}", vehicle_id);
        }

        backend.shutdown().await?;
        println!("\n✓ Simulation complete!");
        println!("\nIsaac Lab Features Demonstrated:");
        println!("  • Parallel environments (4 simultaneous simulations)");
        println!("  • GPU-accelerated physics");
        println!("  • GPU raycasting");
        println!("  • Multi-vehicle coordination");
        println!("  • Sensor integration (IMU, GPS)");
    }

    Ok(())
}
