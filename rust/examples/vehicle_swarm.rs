//! Vehicle simulation example with multiple drones
//!
//! This example demonstrates:
//! - Spawning multiple vehicles
//! - Controlling vehicles
//! - Getting vehicle states
//! - Reading sensor data

use autonomysim_core::native::NativeBackend;
use autonomysim_core::prelude::*;
use autonomysim_core::vehicle::{SensorSpec, SensorType};
use nalgebra::{Point3, UnitQuaternion};

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    println!("=== AutonomySim Multi-Vehicle Simulation ===\n");

    // Create backend
    println!("Initializing backend...");
    let mut backend = NativeBackend::new();
    backend.initialize(BackendConfig::default()).await?;
    println!("✓ Backend initialized\n");

    // Load scene
    let _scene = backend.load_scene("city").await?;
    println!("✓ Scene loaded\n");

    // Spawn multiple drones
    println!("Spawning vehicles...");

    let drone_positions = [Point3::new(0.0, 0.0, 10.0),
        Point3::new(50.0, 0.0, 10.0),
        Point3::new(0.0, 50.0, 10.0),
        Point3::new(50.0, 50.0, 10.0)];

    let mut vehicle_ids = Vec::new();

    for (i, pos) in drone_positions.iter().enumerate() {
        let spec = VehicleSpec {
            vehicle_id: format!("drone{}", i),
            vehicle_type: VehicleType::Multirotor,
            initial_transform: Transform::new(*pos, UnitQuaternion::identity()),
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
        vehicle_ids.push(vehicle_id.clone());
        println!(
            "  ✓ Spawned {} at ({:.1}, {:.1}, {:.1})",
            vehicle_id, pos.x, pos.y, pos.z
        );
    }

    println!("\n✓ {} vehicles spawned\n", vehicle_ids.len());

    // Simulate for a few steps
    println!("Running simulation...");

    for step in 0..10 {
        // Step simulation
        backend.step(0.1).await?;

        if step % 3 == 0 {
            println!("\nStep {}: t = {:.1}s", step, backend.get_time());

            // Set control for each drone
            for vehicle_id in &vehicle_ids {
                let control = VehicleControl::hover();
                backend.set_vehicle_control(vehicle_id, control)?;
            }

            // Get state of first drone
            if let Some(vehicle_id) = vehicle_ids.first() {
                let state = backend.get_vehicle_state(vehicle_id)?;
                println!(
                    "  {}: pos=({:.2}, {:.2}, {:.2}), vel=({:.2}, {:.2}, {:.2})",
                    vehicle_id,
                    state.transform.position.x,
                    state.transform.position.y,
                    state.transform.position.z,
                    state.linear_velocity.x,
                    state.linear_velocity.y,
                    state.linear_velocity.z
                );

                // Get sensor data
                if let Ok(SensorData::Imu(imu)) = backend.get_sensor_data(vehicle_id, "imu") {
                    println!(
                        "    IMU: accel=({:.2}, {:.2}, {:.2})",
                        imu.linear_acceleration.x,
                        imu.linear_acceleration.y,
                        imu.linear_acceleration.z
                    );
                }

                if let Ok(SensorData::Gps(gps)) = backend.get_sensor_data(vehicle_id, "gps") {
                    println!(
                        "    GPS: lat={:.6}, lon={:.6}, alt={:.2}",
                        gps.latitude, gps.longitude, gps.altitude
                    );
                }
            }
        }
    }

    println!("\n\n=== Vehicle Formation Control ===\n");

    // Demo: Move drones in formation
    println!("Moving drones forward...");
    for vehicle_id in &vehicle_ids {
        let control = VehicleControl::forward(0.5);
        backend.set_vehicle_control(vehicle_id, control)?;
    }

    // Simulate more steps
    for _ in 0..5 {
        backend.step(0.1).await?;
    }

    println!("✓ Formation movement complete\n");

    // Get final states
    println!("Final vehicle states:");
    for vehicle_id in &vehicle_ids {
        let state = backend.get_vehicle_state(vehicle_id)?;
        println!(
            "  {}: pos=({:.2}, {:.2}, {:.2}), battery={:.1}%",
            vehicle_id,
            state.transform.position.x,
            state.transform.position.y,
            state.transform.position.z,
            state.battery_level * 100.0
        );
    }

    // Cleanup: Remove vehicles
    println!("\nRemoving vehicles...");
    for vehicle_id in vehicle_ids {
        backend.remove_vehicle(&vehicle_id).await?;
        println!("  ✓ Removed {}", vehicle_id);
    }

    // Shutdown
    backend.shutdown().await?;
    println!("\n✓ Simulation complete!");

    Ok(())
}
