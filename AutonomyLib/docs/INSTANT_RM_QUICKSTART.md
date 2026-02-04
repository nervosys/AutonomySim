# NVIDIA instant-rm Integration - Quick Start Guide

## Overview

This guide shows how to use the **InstantRMBridge** to integrate NVIDIA instant-rm neural radiance maps with AutonomySim's RF Gaussian splatting system.

## Prerequisites

### Required Software

1. **Python 3.8 or higher**
   ```bash
   python --version  # Should be 3.8+
   ```

2. **Mitsuba 3** with CUDA variant
   ```bash
   pip install mitsuba
   ```

3. **instant-rm package**
   ```bash
   git clone https://github.com/NVlabs/instant-rm.git
   cd instant-rm
   pip install -e .
   ```

4. **Verify installation**
   ```python
   import mitsuba as mi
   mi.set_variant('cuda_ad_mono_polarized')
   import instant_rm
   print("instant-rm ready!")
   ```

### Hardware Requirements

- **GPU**: NVIDIA GPU with CUDA support (RTX series recommended)
- **RAM**: 16 GB minimum (32 GB recommended for large scenes)
- **CUDA**: Version 11.0 or higher

## Basic Usage

### 1. Initialize the Bridge

```cpp
#include "communication/InstantRMBridge.hpp"
#include "communication/RFGaussianSplat.hpp"

using namespace autonomysim;

// Create and initialize bridge
InstantRMBridge bridge;
if (!bridge.initialize()) {
    std::cerr << "Failed to initialize instant-rm" << std::endl;
    return false;
}
```

### 2. Configure Measurement Plane

```cpp
// Define where to compute the RF field
InstantRMBridge::MeasurementPlane plane;
plane.center = Vector3r(0, 0, 1.5);        // Center at 1.5m height
plane.normal = Vector3r(0, 0, 1);          // Horizontal plane
plane.size = Vector2r(100, 100);           // 100m x 100m area
plane.resolution_x = 128;                  // 128 cells in X
plane.resolution_y = 128;                  // 128 cells in Y

bridge.setMeasurementPlane(plane);
```

### 3. Configure Transmitter

```cpp
// Set up antenna
InstantRMBridge::AntennaConfig antenna;
antenna.pattern_type = "isotropic";        // Omnidirectional
antenna.gain_dbi = 0.0;                    // 0 dBi gain
antenna.orientation = Vector3r(0, 0, 1);   // Pointing up

bridge.setAntennaConfig(antenna);
```

### 4. Generate Radiance Map

```cpp
// Transmitter parameters
Vector3r tx_position(0, 0, 10);            // 10m height
real_T tx_power_dbm = 30.0;                // 1 Watt (30 dBm)
real_T frequency_hz = 2.4e9;               // 2.4 GHz WiFi

// Compute neural radiance map
auto radiance_map = bridge.computeRadianceMap(
    tx_position,
    tx_power_dbm,
    frequency_hz
);

std::cout << "Map size: " << radiance_map.num_cells_x << "x" 
          << radiance_map.num_cells_y << std::endl;
std::cout << "Compute time: " << bridge.getLastComputeTime() << " ms" << std::endl;
```

### 5. Train Gaussian Representation

```cpp
// Initialize Gaussian splat system
RFGaussianSplat rf_splat;
RFGaussianSplat::RFSplatConfig config;
config.num_gaussians = 5000;               // Start with 5000 Gaussians
config.learning_rate = 0.01;               // Learning rate
config.adaptive_density_control = true;    // Auto refine
config.max_iterations = 1000;              // Training iterations

rf_splat.initialize(config);

// Train from radiance map
rf_splat.trainFromRadianceMap(radiance_map);

std::cout << "Training complete!" << std::endl;
std::cout << "Final Gaussians: " << rf_splat.getNumGaussians() << std::endl;
std::cout << "Loss: " << rf_splat.getCurrentLoss() << " dB²" << std::endl;
```

### 6. Query Signal Strength

```cpp
// Query signal at any 3D position
Vector3r query_position(25, 25, 1.5);
real_T signal_dbm = rf_splat.querySignalStrength(query_position);

std::cout << "Signal at (" << query_position.x() << ", " 
          << query_position.y() << ", " << query_position.z() 
          << "): " << signal_dbm << " dBm" << std::endl;

// Get gradient (direction of signal increase)
Vector3r gradient = rf_splat.queryGradient(query_position);
std::cout << "Gradient: " << gradient.norm() << " dB/m" << std::endl;
```

## Advanced Features

### Multi-Transmitter Scenarios

```cpp
// Multiple transmitters
std::vector<Vector3r> tx_positions = {
    Vector3r(0, 0, 10),
    Vector3r(50, 50, 10),
    Vector3r(100, 0, 10)
};

std::vector<real_T> tx_powers = {30.0, 30.0, 30.0};

auto radiance_map = bridge.computeMultiTxRadianceMap(
    tx_positions,
    tx_powers,
    2.4e9
);
```

### Coverage Analysis

```cpp
// Find all points with signal > -80 dBm
real_T min_signal_dbm = -80.0;
real_T resolution_m = 1.0;  // 1 meter grid

auto coverage = rf_splat.queryCoverageArea(min_signal_dbm, resolution_m);

std::cout << "Coverage points: " << coverage.size() << std::endl;
std::cout << "Coverage area: " << coverage.size() * resolution_m * resolution_m 
          << " m²" << std::endl;
```

### Save/Load Models

```cpp
// Save trained Gaussian model
rf_splat.exportToFile("my_rf_model.bin");

// Load later
RFGaussianSplat loaded_model;
loaded_model.initialize(config);
loaded_model.importFromFile("my_rf_model.bin");

// Save/load radiance maps
bridge.saveRadianceMap(radiance_map, "radiance_map.bin");
bridge.loadRadianceMap(radiance_map, "radiance_map.bin");
```

### Custom Scenes

```cpp
// Load Mitsuba scene XML
InstantRMBridge::SceneConfig scene;
scene.scene_file = "urban_environment.xml";
scene.material_type = "lambertian";  // or "smooth", "backscattering"

bridge.setSceneConfig(scene);

// Add obstacles programmatically
scene.obstacle_positions.push_back(Vector3r(10, 10, 0));
scene.obstacle_sizes.push_back(Vector3r(5, 5, 10));  // 5x5x10m
scene.obstacle_permittivity.push_back(5.0);  // Concrete (εr ≈ 5)
```

### Advanced Ray Tracing

```cpp
InstantRMBridge::TracingConfig tracing;
tracing.num_samples = 2048;           // More samples = better accuracy
tracing.max_depth = 10;               // More bounces = better multipath
tracing.compute_delay_spread = true;  // RMS delay spread
tracing.compute_directions = true;    // Mean DoA/DoD
tracing.use_rbp = false;              // Radiative backpropagation (gradients)
tracing.russian_roulette = 0.95;      // Termination probability

bridge.setTracingConfig(tracing);
```

## Configuration Options

### Antenna Patterns

```cpp
antenna.pattern_type = "isotropic";     // Omnidirectional
antenna.pattern_type = "dipole";        // Half-wave dipole
antenna.pattern_type = "hw_dipole";     // Half-wave dipole
antenna.pattern_type = "tr38901";       // 3GPP TR 38.901 model
```

### Material Types

```cpp
scene.material_type = "smooth";         // Specular reflection (metal)
scene.material_type = "lambertian";     // Diffuse scattering (concrete)
scene.material_type = "backscattering"; // Directive lobe (glass)
```

### Gaussian Training

```cpp
config.num_gaussians = 10000;           // Initial Gaussian count
config.learning_rate = 0.01;            // Gradient descent step
config.adaptive_density_control = true; // Auto split/prune
config.densify_grad_threshold = 0.0002; // Split threshold
config.densify_interval = 100;          // Densify every N iters
config.max_iterations = 2000;           // Training iterations
```

## Performance Tuning

### For Speed

```cpp
// Fast preview
config.num_gaussians = 1000;
config.max_iterations = 200;
tracing.num_samples = 256;
tracing.max_depth = 3;
plane.resolution_x = 64;
plane.resolution_y = 64;
```

### For Accuracy

```cpp
// High quality
config.num_gaussians = 50000;
config.max_iterations = 5000;
tracing.num_samples = 4096;
tracing.max_depth = 10;
plane.resolution_x = 512;
plane.resolution_y = 512;
```

### For Memory

```cpp
// Reduce memory usage
config.num_gaussians = 5000;  // Fewer Gaussians
plane.resolution_x = 128;     // Lower resolution
plane.resolution_y = 128;
```

## Common Issues

### "Failed to import instant_rm module"

**Solution**: Install instant-rm
```bash
pip install mitsuba
git clone https://github.com/NVlabs/instant-rm.git
cd instant-rm
pip install -e .
```

### "Mitsuba variant not cuda_ad_mono_polarized"

**Solution**: Set correct variant
```python
import mitsuba as mi
mi.set_variant('cuda_ad_mono_polarized')
```

### Out of Memory

**Solutions**:
- Reduce `num_gaussians`
- Lower measurement plane resolution
- Decrease `num_samples` in ray tracing
- Use GPU with more VRAM

### Slow Training

**Solutions**:
- Reduce `max_iterations`
- Lower `num_samples`
- Decrease plane resolution
- Use faster GPU

## Examples

See `Examples/InstantRMBridgeExample.cpp` for complete examples:

1. **Basic Workflow**: Generate map, train Gaussians, query
2. **Multi-Transmitter**: Multiple TX scenario
3. **Model Comparison**: instant-rm vs. analytical
4. **Persistence**: Save/load models
5. **Optimization**: Transmitter placement optimization

Build and run:
```bash
cd Examples
# Build via Visual Studio or CMake
./InstantRMBridgeExample.exe
```

## API Reference

### InstantRMBridge Methods

| Method                        | Description                  |
| ----------------------------- | ---------------------------- |
| `initialize()`                | Initialize Python bridge     |
| `computeRadianceMap()`        | Generate neural radiance map |
| `computeMultiTxRadianceMap()` | Multi-transmitter scenario   |
| `setMeasurementPlane()`       | Configure measurement plane  |
| `setAntennaConfig()`          | Configure antenna pattern    |
| `setSceneConfig()`            | Load scene or add obstacles  |
| `setTracingConfig()`          | Configure ray tracing        |
| `saveRadianceMap()`           | Save map to file             |
| `loadRadianceMap()`           | Load map from file           |
| `getLastComputeTime()`        | Get last computation time    |

### RFGaussianSplat Methods

| Method                   | Description                |
| ------------------------ | -------------------------- |
| `initialize()`           | Initialize Gaussian system |
| `trainFromRadianceMap()` | Train from instant-rm map  |
| `querySignalStrength()`  | Get signal at position     |
| `queryGradient()`        | Get signal gradient        |
| `queryCoverageArea()`    | Find coverage region       |
| `renderToPointCloud()`   | Export point cloud         |
| `exportToFile()`         | Save trained model         |
| `importFromFile()`       | Load trained model         |
| `getNumGaussians()`      | Get Gaussian count         |
| `getCurrentLoss()`       | Get training loss          |

## Further Reading

- **instant-rm Paper**: https://research.nvidia.com/publication/2023-10_instant-rm
- **Mitsuba 3 Docs**: https://mitsuba.readthedocs.io
- **3D Gaussian Splatting**: https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/
- **AutonomySim Docs**: `docs/INSTANT_RM_GAUSSIAN_SPLATTING.md`

## Support

For issues:
- instant-rm: https://github.com/NVlabs/instant-rm/issues
- AutonomySim: https://github.com/nervosys/AutonomySim/issues

## License

- instant-rm: NVIDIA Source Code License
- Mitsuba 3: BSD-3-Clause
- AutonomySim: MIT License
