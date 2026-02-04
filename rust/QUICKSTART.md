# Quick Start Guide - AutonomySim Rust

Get started with the Rust implementation of AutonomySim RF propagation in 5 minutes.

## Prerequisites

1. **Install Rust** (if not already installed):
   ```powershell
   # Download and run rustup-init.exe from https://rustup.rs/
   # Or use Windows Package Manager:
   winget install Rustlang.Rustup
   ```

2. **Verify Installation**:
   ```powershell
   rustc --version
   cargo --version
   ```

## Quick Test

### 1. Build the Project

```powershell
cd c:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust
cargo build --release
```

Expected output:
```
   Compiling autonomysim-core v0.1.0
   Compiling autonomysim-rf-core v0.1.0
   ...
   Finished release [optimized] target(s) in 15.23s
```

### 2. Run Tests

```powershell
cargo test --workspace
```

Expected: All 17 tests passing ✅

### 3. Run Example

```powershell
cargo run --example basic_rf_sim
```

Expected output:
```
=== AutonomySim RF Propagation Example ===

Creating native backend...
✓ Backend initialized

Creating scene...
Adding obstacles...
✓ Scene created with 3 objects

Transmitter position: (0.0, 0.0, 10.0)

=== Friis (Free-space) Model ===
  Distance:   10.0m → Path Loss:  52.45 dB, RSSI: -28.45 dBm
  Distance:   50.0m → Path Loss:  66.44 dB, RSSI: -42.44 dBm
  Distance:  100.0m → Path Loss:  72.45 dB, RSSI: -48.45 dBm
  Distance:  200.0m → Path Loss:  78.47 dB, RSSI: -54.47 dBm
...
```

## Basic Usage

### Simple Path Loss Calculation

Create a file `my_rf_sim.rs`:

```rust
use autonomysim_core::prelude::*;
use autonomysim_core::native::NativeBackend;
use autonomysim_rf_core::prelude::*;
use nalgebra::Point3;
use std::sync::Arc;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Create backend
    let mut backend = NativeBackend::new();
    backend.initialize(BackendConfig::default()).await?;
    
    // Configure RF propagation
    let config = PropagationConfig {
        model: PropagationModel::Friis,
        frequency_hz: 2.4e9,      // 2.4 GHz WiFi
        tx_power_dbm: 20.0,       // 20 dBm (100 mW)
        tx_gain_dbi: 3.0,         // 3 dBi antenna
        rx_gain_dbi: 3.0,
        ..Default::default()
    };
    
    // Create engine
    let engine = RFPropagationEngine::new(Arc::new(backend), config);
    
    // Calculate path loss at 100m
    let tx_pos = Point3::new(0.0, 0.0, 10.0);
    let rx_pos = Point3::new(100.0, 0.0, 1.5);
    
    let path_loss = engine.compute_path_loss(tx_pos, rx_pos).await?;
    let rssi = engine.compute_rssi(tx_pos, rx_pos).await?;
    
    println!("Path Loss: {:.2} dB", path_loss);
    println!("RSSI: {:.2} dBm", rssi);
    
    Ok(())
}
```

Run it:
```powershell
cargo add anyhow tokio nalgebra
cargo run --bin my_rf_sim
```

### Using Different Propagation Models

```rust
// Free-space (line-of-sight)
let config = PropagationConfig {
    model: PropagationModel::Friis,
    ..Default::default()
};

// Urban environment with ground reflection
let config = PropagationConfig {
    model: PropagationModel::TwoRay,
    ..Default::default()
};

// Empirical log-distance model
let config = PropagationConfig {
    model: PropagationModel::LogDistance,
    path_loss_exponent: 2.7,  // Urban environment
    ..Default::default()
};

// Ray tracing with scene geometry
let config = PropagationConfig {
    model: PropagationModel::RayTracing,
    num_rays: 1000,
    max_reflections: 5,
    ..Default::default()
};
```

### Working with Antennas

```rust
use autonomysim_rf_core::prelude::*;
use nalgebra::Vector3;

// Isotropic antenna (omnidirectional)
let isotropic = Antenna::isotropic();

// Dipole antenna
let dipole = Antenna::dipole();

// Directional antenna (e.g., Yagi)
let directional = Antenna::directional(60.0, 60.0);  // 60° beamwidth

// Calculate effective gain in a direction
let direction = Vector3::new(1.0, 0.0, 0.0);
let gain = directional.effective_gain(direction);
println!("Gain in direction: {:.2} dBi", gain);
```

### Link Budget Analysis

```rust
use autonomysim_rf_core::utils::*;

let tx_power = 20.0;     // dBm
let tx_gain = 3.0;       // dBi
let rx_gain = 3.0;       // dBi
let path_loss = 80.0;    // dB
let system_loss = 2.0;   // dB

let rx_power = link_budget(tx_power, tx_gain, rx_gain, path_loss, system_loss);
println!("Received Power: {:.2} dBm", rx_power);

// Calculate SNR
let bandwidth = 20e6;    // 20 MHz
let temperature = 290.0; // K
let noise = thermal_noise_dbm(temperature, bandwidth);
let snr = calculate_snr(rx_power, noise);

println!("Noise: {:.2} dBm", noise);
println!("SNR: {:.2} dB", snr);
```

### Creating a Scene with Obstacles

```rust
use autonomysim_core::prelude::*;
use nalgebra::{Point3, UnitQuaternion, Vector3};

// Create a building
let building = SceneObject {
    id: "building1".to_string(),
    name: "Office Building".to_string(),
    transform: Transform::new(
        Point3::new(50.0, 0.0, 10.0),
        UnitQuaternion::identity(),
    ),
    geometry: Geometry::Box {
        size: Vector3::new(20.0, 30.0, 20.0),
    },
    material: Material::concrete(),
};

// Add to scene
backend.add_object(&scene, building)?;
```

## Common Patterns

### Compute Coverage Map

```rust
async fn compute_coverage_map(
    engine: &RFPropagationEngine,
    tx_pos: Point3<f64>,
    grid_size: f64,
    range: f64,
) -> Vec<(Point3<f64>, f64)> {
    let mut coverage = Vec::new();
    
    for x in (-range..range).step_by(grid_size as usize) {
        for y in (-range..range).step_by(grid_size as usize) {
            let rx_pos = Point3::new(x as f64, y as f64, 1.5);
            
            if let Ok(rssi) = engine.compute_rssi(tx_pos, rx_pos).await {
                coverage.push((rx_pos, rssi));
            }
        }
    }
    
    coverage
}
```

### Multi-Transmitter Scenario

```rust
let transmitters = vec![
    Point3::new(0.0, 0.0, 10.0),
    Point3::new(100.0, 100.0, 10.0),
    Point3::new(200.0, 0.0, 10.0),
];

let rx_pos = Point3::new(150.0, 50.0, 1.5);

let mut total_power_linear = 0.0;
for tx_pos in transmitters {
    let rssi_dbm = engine.compute_rssi(tx_pos, rx_pos).await?;
    let power_watts = dbm_to_watts(rssi_dbm);
    total_power_linear += power_watts;
}

let total_rssi = watts_to_dbm(total_power_linear);
println!("Combined RSSI: {:.2} dBm", total_rssi);
```

## Troubleshooting

### Build Errors

**Problem**: "failed to parse manifest"
```
Solution: Make sure you're in the rust/ directory
cd c:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust
```

**Problem**: "linker error"
```
Solution: Install Visual Studio Build Tools or ensure MSVC is available
```

### Runtime Errors

**Problem**: "Backend not initialized"
```rust
// Always initialize before use
backend.initialize(BackendConfig::default()).await?;
```

**Problem**: "Scene not found"
```rust
// Set scene before computing RF
engine.set_scene(scene);
```

## Next Steps

1. **Read the README**: Full documentation in `README.md`
2. **Explore Examples**: Check `examples/basic_rf_sim.rs`
3. **API Documentation**: Run `cargo doc --open`
4. **Write Tests**: Add to `tests/` directory
5. **Benchmarks**: Create in `benches/` directory

## Useful Commands

```powershell
# Build (debug mode)
cargo build

# Build (release mode - optimized)
cargo build --release

# Run tests
cargo test

# Run specific test
cargo test test_friis_path_loss

# Run example
cargo run --example basic_rf_sim

# Generate documentation
cargo doc --open

# Check for errors (fast)
cargo check

# Format code
cargo fmt

# Lint code
cargo clippy

# Update dependencies
cargo update

# Clean build artifacts
cargo clean
```

## Performance Tips

1. **Always use release mode for benchmarks**:
   ```powershell
   cargo build --release
   cargo run --release --example basic_rf_sim
   ```

2. **Enable parallel processing**:
   ```rust
   let config = BackendConfig {
       parallel_processing: true,
       num_threads: Some(8),
       ..Default::default()
   };
   ```

3. **Use appropriate ray counts**:
   ```rust
   // Fast but less accurate
   num_rays: 100,
   
   // Balanced
   num_rays: 1000,
   
   // Accurate but slower
   num_rays: 10000,
   ```

## Integration with Existing C++ Code

The Rust implementation can coexist with the C++ code. Future FFI bindings will allow:

```cpp
// C++ calling Rust (planned Phase 4)
#include "autonomysim_rust_bindings.h"

auto backend = autonomysim_create_backend(BACKEND_TYPE_NATIVE);
double path_loss = autonomysim_compute_path_loss(
    backend, tx_pos, rx_pos, 2.4e9
);
```

## Resources

- **Rust Book**: https://doc.rust-lang.org/book/
- **Cargo Guide**: https://doc.rust-lang.org/cargo/
- **Tokio Tutorial**: https://tokio.rs/tokio/tutorial
- **nalgebra Docs**: https://nalgebra.org/
- **AutonomySim C++**: `../AutonomyLib/` directory

## Support

- Check `STATUS.md` for current implementation status
- See `README.md` for detailed documentation
- Review examples in `examples/` directory
- Run tests for code examples: `cargo test`

---

**Ready to get started?** Run the example:
```powershell
cd c:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust
cargo run --example basic_rf_sim
```
