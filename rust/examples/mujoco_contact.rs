//! MuJoCo Backend Example - Contact-Rich Simulation
//!
//! Demonstrates advanced contact dynamics and precise physics simulation.
//!
//! Run with: cargo run --example mujoco_contact --features mujoco

use anyhow::Result;
use autonomysim_core::{
    backend::{BackendConfig, SimulationBackend},
    vehicle::{SensorSpec, SensorType, VehicleControl, VehicleSpec, VehicleType},
    Transform,
};
use nalgebra::{Point3, UnitQuaternion};

#[cfg(feature = "mujoco")]
use autonomysim_backends::MuJoCoBackend;

#[tokio::main]
async fn main() -> Result<()> {
    println!("=== MuJoCo Contact-Rich Simulation ===\n");

    #[cfg(not(feature = "mujoco"))]
    {
        println!("❌ This example requires the 'mujoco' feature");
        println!("   Run with: cargo run --example mujoco_contact --features mujoco");
        return Ok(());
    }

    #[cfg(feature = "mujoco")]
    {
        // Initialize MuJoCo with fine timestep for accurate contact dynamics
        println!("Initializing MuJoCo backend...");
        println!("  • Timestep: 2ms (500Hz)");
        println!("  • Contact detection: Enabled");
        println!("  • Solver iterations: 100");

        let mut backend = MuJoCoBackend::with_config(autonomysim_backends::mujoco::MuJoCoConfig {
            timestep: 0.002, // 2ms for precise contact
            substeps: 1,
            enable_contact: true,
            enable_limits: true,
            solver_iterations: 100, // High accuracy
            model_path: None,
        });

        backend.initialize(BackendConfig::default()).await?;
        println!("✓ Backend initialized\n");

        // Load scene with contact surfaces
        println!("Loading scene with contact surfaces...");
        let _scene = backend.load_scene("terrain_with_obstacles.xml").await?;
        println!("✓ Scene loaded\n");

        // Spawn ground vehicle for contact-rich simulation
        println!("Spawning ground vehicle...");
        let car_spec = VehicleSpec {
            vehicle_id: "car_1".to_string(),
            vehicle_type: VehicleType::Car,
            initial_transform: Transform::new(
                Point3::new(0.0, 0.0, 0.5), // Start 0.5m above ground
                UnitQuaternion::identity(),
            ),
            parameters: Default::default(),
            sensors: vec![
                SensorSpec {
                    sensor_id: "imu".to_string(),
                    sensor_type: SensorType::Imu,
                    update_rate_hz: 500.0, // High rate for contact events
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

        let car_id = backend.spawn_vehicle(car_spec).await?;
        println!("✓ Spawned {} (ground vehicle)\n", car_id);

        // Spawn a multirotor for comparison
        println!("Spawning aerial vehicle...");
        let drone_spec = VehicleSpec {
            vehicle_id: "drone_1".to_string(),
            vehicle_type: VehicleType::Multirotor,
            initial_transform: Transform::new(
                Point3::new(5.0, 0.0, 2.0),
                UnitQuaternion::identity(),
            ),
            parameters: Default::default(),
            sensors: vec![SensorSpec {
                sensor_id: "imu".to_string(),
                sensor_type: SensorType::Imu,
                update_rate_hz: 200.0,
                enabled: true,
            }],
        };

        let drone_id = backend.spawn_vehicle(drone_spec).await?;
        println!("✓ Spawned {} (quadrotor)\n", drone_id);

        // Simulate contact scenario
        println!("=== Contact Simulation ===\n");
        println!("Simulating vehicle driving over rough terrain...");
        println!("MuJoCo excels at contact-rich scenarios:\n");

        for step in 0..20 {
            // Step with substeps for contact stability
            backend.step(0.05).await?; // 50ms per step, but internally using 2ms timesteps

            // Drive forward
            if step < 15 {
                backend.set_vehicle_control(
                    &car_id,
                    VehicleControl {
                        throttle: 0.5,
                        steering: if step > 10 { 0.3 } else { 0.0 }, // Turn after 10 steps
                        brake: 0.0,
                        pitch: 0.0,
                        roll: 0.0,
                        yaw: 0.0,
                        manual_gear: None,
                        is_manual_gear: false,
                    },
                )?;
            } else {
                // Brake
                backend.set_vehicle_control(
                    &car_id,
                    VehicleControl {
                        throttle: 0.0,
                        steering: 0.0,
                        brake: 1.0,
                        pitch: 0.0,
                        roll: 0.0,
                        yaw: 0.0,
                        manual_gear: None,
                        is_manual_gear: false,
                    },
                )?;
            }

            // Hover drone
            backend.set_vehicle_control(&drone_id, VehicleControl::hover())?;

            // Print state every 5 steps
            if step % 5 == 0 {
                let car_state = backend.get_vehicle_state(&car_id)?;
                let drone_state = backend.get_vehicle_state(&drone_id)?;

                println!("Step {}: t = {:.2}s", step, backend.get_time());
                println!(
                    "  Car: pos=({:.2}, {:.2}, {:.2}) grounded={}",
                    car_state.transform.position.x,
                    car_state.transform.position.y,
                    car_state.transform.position.z,
                    car_state.is_grounded
                );
                println!(
                    "  Drone: pos=({:.2}, {:.2}, {:.2})",
                    drone_state.transform.position.x,
                    drone_state.transform.position.y,
                    drone_state.transform.position.z
                );

                // Read IMU data - shows contact forces
                let imu_data = backend.get_sensor_data(&car_id, "imu")?;
                if let autonomysim_core::sensor::SensorData::Imu(imu) = imu_data {
                    println!(
                        "  IMU: accel=({:.2}, {:.2}, {:.2})",
                        imu.linear_acceleration.x,
                        imu.linear_acceleration.y,
                        imu.linear_acceleration.z
                    );
                }
                println!();
            }
        }

        println!("\n=== Contact Features ===\n");
        println!("MuJoCo's strengths demonstrated:");
        println!("  • Precise contact detection and resolution");
        println!("  • Stable constraint-based solver");
        println!("  • Joint limits and friction modeling");
        println!("  • Efficient substep integration");
        println!("  • Ground contact forces (wheel-terrain interaction)");

        println!("\n=== Ray Casting Test ===\n");
        let scene = backend.load_scene("terrain_with_obstacles.xml").await?;

        // Cast rays downward from both vehicles
        println!("Casting rays to detect ground distance...");
        for (vehicle_id, name) in [(&car_id, "Car"), (&drone_id, "Drone")] {
            let state = backend.get_vehicle_state(vehicle_id)?;
            let ray = autonomysim_core::Ray {
                origin: state.transform.position,
                direction: nalgebra::Vector3::new(0.0, 0.0, -1.0),
                max_distance: 10.0,
            };

            if let Some(hit) = backend.cast_ray(&scene, &ray)? {
                println!("  {}: Ground {:.2}m below", name, hit.distance);
            }
        }

        println!("\n=== Cleanup ===\n");
        backend.remove_vehicle(&car_id).await?;
        println!("✓ Removed {}", car_id);
        backend.remove_vehicle(&drone_id).await?;
        println!("✓ Removed {}", drone_id);

        backend.shutdown().await?;
        println!("\n✓ Simulation complete!");

        println!("\nMuJoCo Features:");
        println!("  • Contact dynamics: Wheel-ground interaction");
        println!("  • Precise timestep: 2ms (500Hz)");
        println!("  • Constraint solver: 100 iterations");
        println!("  • Vehicle types: Car, Quadrotor");
        println!("  • Sensors: IMU (contact forces), GPS");
    }

    Ok(())
}
