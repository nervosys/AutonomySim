//! Basic RF propagation simulation example
//!
//! This example demonstrates:
//! - Creating a native simulation backend
//! - Loading/creating a simple scene
//! - Computing RF path loss using different models
//! - Calculating RSSI at various receiver positions

use autonomysim_core::native::NativeBackend;
use autonomysim_core::prelude::*;
use autonomysim_rf_core::prelude::*;
use nalgebra::{Point3, UnitQuaternion, Vector3};
use std::sync::Arc;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    println!("=== AutonomySim RF Propagation Example ===\n");

    // Create and initialize backend
    println!("Creating native backend...");
    let mut backend = BackendFactory::create(BackendType::Native)?;
    backend.initialize(BackendConfig::default()).await?;
    println!("✓ Backend initialized\n");

    // Load/create scene
    println!("Creating scene...");
    let scene = backend.load_scene("example_scene").await?;

    // Add some obstacles to the scene
    println!("Adding obstacles...");

    // Ground plane
    let ground = SceneObject {
        id: "ground".to_string(),
        name: "Ground Plane".to_string(),
        transform: Transform::new(Point3::new(0.0, 0.0, -0.5), UnitQuaternion::identity()),
        geometry: Geometry::Box {
            size: Vector3::new(1000.0, 1000.0, 1.0),
        },
        material: Material {
            name: "Concrete".to_string(),
            permittivity: 5.0,
            conductivity: 0.01,
            permeability: 1.0,
            roughness: 0.001,
            reflection_coefficient: 0.3,
        },
    };
    backend.add_object(&scene, ground)?;

    // Building 1
    let building1 = SceneObject {
        id: "building1".to_string(),
        name: "Building 1".to_string(),
        transform: Transform::new(Point3::new(50.0, 0.0, 10.0), UnitQuaternion::identity()),
        geometry: Geometry::Box {
            size: Vector3::new(20.0, 30.0, 20.0),
        },
        material: Material::concrete(),
    };
    backend.add_object(&scene, building1)?;

    // Building 2
    let building2 = SceneObject {
        id: "building2".to_string(),
        name: "Building 2".to_string(),
        transform: Transform::new(Point3::new(80.0, 40.0, 15.0), UnitQuaternion::identity()),
        geometry: Geometry::Box {
            size: Vector3::new(25.0, 25.0, 30.0),
        },
        material: Material::concrete(),
    };
    backend.add_object(&scene, building2)?;

    println!(
        "✓ Scene created with {} objects\n",
        backend.get_objects(&scene)?.len()
    );

    // Transmitter position
    let tx_pos = Point3::new(0.0, 0.0, 10.0);
    println!(
        "Transmitter position: ({:.1}, {:.1}, {:.1})\n",
        tx_pos.x, tx_pos.y, tx_pos.z
    );

    // Test different propagation models
    let models = vec![
        (PropagationModel::Friis, "Friis (Free-space)"),
        (PropagationModel::TwoRay, "Two-Ray Ground Reflection"),
        (PropagationModel::LogDistance, "Log-Distance"),
        // Note: RayTracing requires backend to be Arc - skip for now
    ];

    for (model, model_name) in models {
        println!("=== {} Model ===", model_name);

        let config = PropagationConfig {
            model,
            frequency_hz: 2.4e9,
            tx_power_dbm: 20.0,
            tx_gain_dbi: 3.0,
            rx_gain_dbi: 3.0,
            system_loss_db: 2.0,
            path_loss_exponent: 2.7, // Urban environment
            ..Default::default()
        };

        // Create a new native backend for each model
        let mut model_backend = NativeBackend::new();
        model_backend.initialize(BackendConfig::default()).await?;
        let model_scene = model_backend.load_scene("test_scene").await?;

        // Re-add objects
        for obj in backend.get_objects(&scene)? {
            model_backend.add_object(&model_scene, obj)?;
        }

        let backend_arc = Arc::new(model_backend);
        let mut engine = RFPropagationEngine::new(backend_arc, config);
        engine.set_scene(model_scene);

        // Test at different distances
        let distances = vec![10.0, 50.0, 100.0, 200.0];

        for distance in distances {
            let rx_pos = Point3::new(distance, 0.0, 1.5);

            match engine.compute_path_loss(tx_pos, rx_pos).await {
                Ok(path_loss) => {
                    let rssi = engine.compute_rssi(tx_pos, rx_pos).await?;
                    println!(
                        "  Distance: {:>6.1}m → Path Loss: {:>6.2} dB, RSSI: {:>7.2} dBm",
                        distance, path_loss, rssi
                    );
                }
                Err(e) => {
                    println!("  Distance: {:>6.1}m → Error: {}", distance, e);
                }
            }
        }

        println!();
    }

    // Antenna pattern example
    println!("=== Antenna Pattern Example ===");
    let antennas = vec![
        (Antenna::isotropic(), "Isotropic"),
        (Antenna::dipole(), "Dipole"),
        (
            Antenna::directional(60.0, 60.0),
            "Directional (60° beamwidth)",
        ),
    ];

    for (antenna, name) in antennas {
        println!("{}:", name);
        println!("  Gain: {:.2} dBi", antenna.gain_dbi);

        // Calculate effective gain at different angles
        let directions = vec![
            (Vector3::new(0.0, 0.0, 1.0), "Boresight"),
            (Vector3::new(1.0, 0.0, 0.0), "90° off-axis"),
            (Vector3::new(0.0, 0.0, -1.0), "Back lobe"),
        ];

        for (direction, label) in directions {
            let eff_gain = antenna.effective_gain(direction);
            println!("  {} gain: {:.2} dBi", label, eff_gain);
        }
        println!();
    }

    // Link budget calculation
    println!("=== Link Budget Analysis ===");
    let tx_power = 20.0;
    let tx_gain = 3.0;
    let rx_gain = 3.0;
    let path_loss = 80.0;
    let system_loss = 2.0;

    let received_power = link_budget(tx_power, tx_gain, rx_gain, path_loss, system_loss);

    println!("Transmit Power:     {:>7.2} dBm", tx_power);
    println!("Tx Antenna Gain:    {:>7.2} dBi", tx_gain);
    println!("Rx Antenna Gain:    {:>7.2} dBi", rx_gain);
    println!("Path Loss:          {:>7.2} dB", path_loss);
    println!("System Losses:      {:>7.2} dB", system_loss);
    println!("─────────────────────────────────");
    println!("Received Power:     {:>7.2} dBm", received_power);

    // Calculate noise and SNR
    let bandwidth = 20e6; // 20 MHz
    let temperature = 290.0; // Room temperature
    let noise_power = thermal_noise_dbm(temperature, bandwidth);
    let snr = calculate_snr(received_power, noise_power);

    println!("\nNoise Power:        {:>7.2} dBm", noise_power);
    println!("SNR:                {:>7.2} dB", snr);

    // Shutdown backend
    backend.shutdown().await?;
    println!("\n✓ Simulation complete!");

    Ok(())
}
