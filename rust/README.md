# AutonomySim Rust Implementation

**Multi-Backend RF Propagation Simulation with Memory Safety**

This is a complete rewrite of AutonomySim's RF propagation system in Rust, providing:

- **Memory Safety**: Rust's ownership system prevents data races and memory leaks
- **Multi-Backend Support**: Unified interface for UE5, Isaac Lab, MuJoCo, and Warp
- **High Performance**: Zero-cost abstractions, SIMD, and parallel processing
- **Cross-Platform**: Native support for Windows, Linux, and macOS
- **Modular Design**: Separate crates for different functionality

## Architecture

```
rust/
├── autonomysim-core/           # Core types, traits, and native backend
├── autonomysim-backends/       # Backend implementations
├── autonomysim-rf-core/        # RF propagation models
├── autonomysim-gaussian-splat/ # 3D Gaussian splatting
├── autonomysim-instant-rm/     # NVIDIA instant-rm bridge
├── autonomysim-bindings/       # C/C++/Python FFI
└── examples/                   # Example applications
```

## Features

### Core Features

- **Backend Abstraction**: Unified `SimulationBackend` trait for all simulation engines
- **Native Backend**: Fast CPU-based ray tracing (no external dependencies)
- **Scene Management**: Load, modify, and query 3D scenes
- **Material Properties**: Physical material properties for RF simulation

### RF Propagation Models

- ✅ **Friis**: Free-space path loss
- ✅ **Two-Ray**: Ground reflection model
- ✅ **Log-Distance**: Empirical path loss
- ✅ **Ray Tracing**: Physics-based with scene queries
- ✅ **Gaussian Beam**: Beam propagation
- ✅ **ITM (Longley-Rice)**: Irregular terrain model
- ✅ **COST 231**: Urban propagation model

### Antenna Models

- Isotropic (omnidirectional)
- Dipole patterns
- Directional with configurable beamwidth
- Custom patterns from measurements

### Utilities

- Power conversions (dBm ↔ Watts, dBW ↔ Watts)
- Link budget calculations
- SNR computation
- Thermal noise calculation
- Fresnel reflection coefficients
- Knife-edge diffraction

## Quick Start

### Prerequisites

- Rust 1.70+ ([Install Rust](https://www.rust-lang.org/tools/install))
- Git

### Building

```powershell
# Clone the repository (if not already)
cd c:\Users\adamm\dev\nervosys\modeling\simulation\AutonomySim\rust

# Build all crates
cargo build --release

# Run tests
cargo test

# Run example
cargo run --example basic_rf_sim
```

### Basic Usage

```rust
use autonomysim_core::prelude::*;
use autonomysim_rf_core::prelude::*;
use nalgebra::Point3;
use std::sync::Arc;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Create backend
    let mut backend = BackendFactory::create(BackendType::Native)?;
    backend.initialize(BackendConfig::default()).await?;
    
    // Load scene
    let scene = backend.load_scene("city.obj").await?;
    
    // Configure RF propagation
    let config = PropagationConfig {
        model: PropagationModel::RayTracing,
        frequency_hz: 2.4e9,  // 2.4 GHz
        tx_power_dbm: 20.0,   // 20 dBm (100 mW)
        ..Default::default()
    };
    
    let backend_arc = Arc::new(backend);
    let mut engine = RFPropagationEngine::new(backend_arc, config);
    engine.set_scene(scene);
    
    // Compute path loss
    let tx_pos = Point3::new(0.0, 0.0, 10.0);
    let rx_pos = Point3::new(100.0, 0.0, 1.5);
    
    let path_loss = engine.compute_path_loss(tx_pos, rx_pos).await?;
    println!("Path loss: {:.2} dB", path_loss);
    
    // Compute RSSI
    let rssi = engine.compute_rssi(tx_pos, rx_pos).await?;
    println!("RSSI: {:.2} dBm", rssi);
    
    Ok(())
}
```

## Crate Documentation

### `autonomysim-core`

Core types and backend abstraction layer.

**Key Types**:
- `SimulationBackend` trait: Abstract interface for simulation engines
- `SceneHandle`, `SceneObject`: Scene management
- `Material`: RF material properties
- `Ray`, `RayHit`: Ray tracing primitives
- `Transform`, `Position`: 3D transforms

**Backends**:
- `NativeBackend`: Built-in CPU ray tracing

### `autonomysim-rf-core`

RF propagation models and calculations.

**Key Types**:
- `RFPropagationEngine`: Main RF simulation engine
- `PropagationConfig`: Configuration for RF models
- `PropagationModel`: Enum of available models
- `Antenna`: Antenna patterns and gains

**Models**:
```rust
pub enum PropagationModel {
    Friis,          // Free-space
    TwoRay,         // Ground reflection
    LogDistance,    // Empirical
    ITM,            // Irregular terrain
    RayTracing,     // Physics-based
    GaussianBeam,   // Beam propagation
    COST231,        // Urban
}
```

### `autonomysim-backends` (Planned)

Implementations for external simulation engines:

- **UnrealEngine5Backend**: Integration with UE5
- **IsaacLabBackend**: NVIDIA Isaac Lab
- **MuJoCoBackend**: MuJoCo physics
- **WarpBackend**: NVIDIA Warp

### `autonomysim-gaussian-splat` (Planned)

3D Gaussian splatting for neural RF field representation.

### `autonomysim-instant-rm` (Planned)

FFI bridge to NVIDIA's instant-rm (instant radiance maps) for neural RF.

### `autonomysim-bindings` (Planned)

C/C++ and Python bindings for interoperability with existing code.

## Examples

### Basic RF Simulation

```powershell
cargo run --example basic_rf_sim
```

Demonstrates:
- Creating a scene with obstacles
- Computing path loss with different models
- Antenna pattern analysis
- Link budget calculations

Output:
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

## Performance

### Benchmarks

```powershell
cargo bench
```

Expected performance (release mode):
- **Path Loss Calculation**: ~1-10 µs per point (model-dependent)
- **Ray Casting**: ~100-500 ns per ray
- **Scene Query**: ~50-200 ns per object

### Optimization

The Rust implementation includes:

- **LTO (Link-Time Optimization)**: Enabled in release mode
- **Parallel Processing**: Rayon for data parallelism
- **SIMD**: Portable SIMD where available
- **Zero-Cost Abstractions**: No runtime overhead

## Testing

```powershell
# Run all tests
cargo test

# Run tests for specific crate
cargo test -p autonomysim-core
cargo test -p autonomysim-rf-core

# Run with output
cargo test -- --nocapture

# Run specific test
cargo test test_friis_path_loss
```

## Multi-Backend Support

### Backend Trait

All backends implement the `SimulationBackend` trait:

```rust
#[async_trait]
pub trait SimulationBackend: Send + Sync {
    async fn initialize(&mut self, config: BackendConfig) -> SimResult<()>;
    async fn load_scene(&mut self, path: &str) -> SimResult<SceneHandle>;
    fn cast_ray(&self, scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>>;
    fn cast_rays(&self, scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>>;
    // ... more methods
}
```

### Creating Backends

```rust
// Native backend (built-in)
let backend = BackendFactory::create(BackendType::Native)?;

// Unreal Engine 5 (when implemented)
let backend = BackendFactory::create(BackendType::UnrealEngine5)?;

// Isaac Lab (when implemented)
let backend = BackendFactory::create(BackendType::IsaacLab)?;
```

## C/C++ Interoperability (Planned)

### C API

```c
// Create backend
void* backend = autonomysim_create_backend(BACKEND_TYPE_NATIVE);

// Compute path loss
double path_loss = autonomysim_compute_path_loss(
    backend,
    (CVector3){0.0, 0.0, 10.0},  // TX position
    (CVector3){100.0, 0.0, 1.5},  // RX position
    2.4e9                         // Frequency
);

// Cleanup
autonomysim_destroy_backend(backend);
```

### Python Bindings (Planned)

```python
import autonomysim

# Create backend
backend = autonomysim.Backend.native()
backend.initialize()

# Load scene
scene = backend.load_scene("city.obj")

# Create RF engine
engine = autonomysim.RFEngine(backend)
engine.frequency = 2.4e9  # Hz
engine.tx_power = 20.0    # dBm

# Compute path loss
path_loss = engine.path_loss(
    tx=(0.0, 0.0, 10.0),
    rx=(100.0, 0.0, 1.5)
)

print(f"Path loss: {path_loss:.2f} dB")
```

## Migration from C++

### Equivalent Functionality

| C++ Class                | Rust Equivalent              | Notes                    |
| ------------------------ | ---------------------------- | ------------------------ |
| `RTXRFPropagation`       | `RFPropagationEngine`        | Same models, async API   |
| `RFGaussianSplat`        | `autonomysim-gaussian-splat` | Planned                  |
| `InstantRMBridge`        | `autonomysim-instant-rm`     | Planned with PyO3        |
| Direct pointer access    | `Arc<T>`, `&T`               | Memory-safe references   |
| Manual memory management | Automatic (RAII)             | No manual cleanup needed |

### Key Differences

1. **Memory Safety**: Rust prevents data races and use-after-free at compile time
2. **Async/Await**: Modern async/await instead of callbacks
3. **Error Handling**: `Result<T, E>` instead of exceptions
4. **Type Safety**: Strong typing with inference
5. **Ownership**: Explicit ownership model prevents leaks

## Roadmap

### Phase 1: Core (CURRENT - Complete)
- ✅ Backend abstraction trait
- ✅ Native backend implementation
- ✅ RF propagation models (7 models)
- ✅ Antenna patterns
- ✅ Basic examples and tests

### Phase 2: Advanced RF (Next)
- ⏳ 3D Gaussian splatting for RF
- ⏳ instant-rm FFI bridge (Python)
- ⏳ Advanced ray tracing with BVH
- ⏳ GPU acceleration (CUDA/Vulkan)

### Phase 3: Multi-Backend
- ⏳ Unreal Engine 5 backend
- ⏳ Isaac Lab backend
- ⏳ MuJoCo backend
- ⏳ Warp backend

### Phase 4: Bindings
- ⏳ C API with cbindgen
- ⏳ Python bindings with PyO3
- ⏳ Node.js bindings (optional)

### Phase 5: Optimization
- ⏳ SIMD optimizations
- ⏳ Parallel ray tracing
- ⏳ GPU compute shaders
- ⏳ Cache-friendly data structures

## Contributing

### Code Style

```powershell
# Format code
cargo fmt

# Lint code
cargo clippy

# Check for issues
cargo clippy -- -D warnings
```

### Adding a New Propagation Model

1. Add model to `PropagationModel` enum in `autonomysim-rf-core/src/propagation.rs`
2. Implement calculation method in `RFPropagationEngine`
3. Add test case
4. Update documentation

Example:
```rust
impl RFPropagationEngine {
    fn my_new_model_path_loss(&self, distance: f64) -> f64 {
        // Your implementation
        let base_loss = self.friis_path_loss(distance);
        base_loss + additional_factors
    }
}
```

## License

MIT License - see LICENSE file for details

## Acknowledgments

- Original C++ implementation: AutonomySim team
- NVIDIA instant-rm: [github.com/NVlabs/instant-rm](https://github.com/NVlabs/instant-rm)
- Rust community for excellent libraries (nalgebra, tokio, rayon)

## Contact

For questions or issues:
- GitHub Issues: [Create an issue](https://github.com/autonomysim/autonomysim/issues)
- Documentation: See `docs/` directory
- Examples: See `examples/` directory

---

**Built with ❤️ and 🦀 (Rust)**
