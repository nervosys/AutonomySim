# NVIDIA instant-rm + 3D Gaussian Splatting Integration

## Overview

This document describes the integration of NVIDIA instant-rm (instant radio maps) with 3D Gaussian Splatting for real-time, differentiable RF propagation visualization in AutonomySim.

### Key Features

- **Neural Radiance Maps**: Fast differentiable radio map computation using ray tube tracing
- **3D Gaussian Splatting**: Efficient neural representation of RF fields
- **Real-Time Performance**: 100+ radio maps/second via GPU acceleration
- **Differentiable**: Full gradient support for optimization
- **Adaptive Density Control**: Automatic Gaussian refinement during training

## Architecture

### instant-rm (Radio Map Generation)

instant-rm is NVIDIA's differentiable ray tracer for radio propagation, based on Mitsuba 3 and Dr.Jit.

**Core Components**:

```
instant-rm
├── MapTracer: Ray tracing for radio maps
│   ├── Fibonacci lattice sampling
│   ├── Ray tube propagation
│   ├── Mueller matrix handling (polarization)
│   └── Measurement plane intersection
├── PathlossMapRBPTracer: Radiative backpropagation
│   ├── Forward pass with end-to-end matrices
│   ├── Gradient computation via chain rule
│   └── Dr.Jit loop recording for speed
├── RayTube: Electromagnetic field representation
│   ├── Electric field (Stokes vector)
│   ├── Principal radii of curvature (ρ₁, ρ₂)
│   ├── Solid angle
│   └── Path length
├── Materials (BSDF)
│   ├── SmoothMaterial: Specular reflection
│   ├── LambertianMaterial: Diffuse scattering
│   └── BackscatteringMaterial: Directive lobe
└── Antenna Patterns
    ├── Isotropic
    ├── Dipole
    ├── Half-wave dipole
    └── TR38.901 models
```

**Performance**: ~100 radio maps/second on NVIDIA RTX 4090

### 3D Gaussian Splatting (Neural Representation)

Represents RF fields as a collection of 3D Gaussians with anisotropic covariance matrices.

**Gaussian Structure**:
```cpp
struct Gaussian3D {
    Vector3r position;           // Center position (x, y, z)
    Vector3r scale;              // Scale along principal axes
    Quaternionr rotation;        // Orientation
    real_T signal_strength_dbm;  // RF signal strength
    real_T frequency_hz;         // Frequency
    real_T opacity;              // Visibility (α)
    Vector3r color_rgb;          // Visualization color
    real_T covariance[6];        // 3D covariance matrix (upper triangular)
};
```

**Covariance Matrix**:
```
Σ = R · S · Sᵀ · Rᵀ

Where:
- R: Rotation matrix from quaternion
- S: Diagonal scale matrix
```

**Gaussian Evaluation**:
```
G(x) = α · exp(-0.5 · (x - μ)ᵀ · Σ⁻¹ · (x - μ))

Where:
- α: Opacity
- μ: Position
- Σ: Covariance matrix
- x: Query point
```

## Workflow

### 1. Generate Neural Radiance Map (instant-rm)

```python
import mitsuba as mi
import drjit as dr
from instant_rm import MapTracer

# Load scene
scene = mi.load_file('environment.xml')

# Configure transmitter
tx_position = [0, 0, 2]
tx_power = 20.0  # dBm
frequency = 2.4e9  # 2.4 GHz

# Define measurement plane
measurement_plane = {
    'center': [0, 0, 1],
    'orientation': [0, 0, 1],
    'size': [100, 100],  # meters
    'resolution': [256, 256]  # cells
}

# Create tracer
tracer = MapTracer(
    scene=scene,
    antenna_pattern='isotropic',
    num_samples=1024,
    max_depth=5
)

# Trace radio map
radiance_map = tracer(
    tx_position=tx_position,
    tx_power=tx_power,
    frequency=frequency,
    measurement_plane=measurement_plane
)

# radiance_map contains:
# - path_loss_map: (256, 256) array
# - rms_delay_spread: (256, 256) array
# - mean_direction_arrival: (256, 256, 3) array
# - mean_direction_departure: (256, 256, 3) array
```

### 2. Train Gaussian Representation (C++)

```cpp
#include "communication/RFGaussianSplat.hpp"

using namespace autonomysim;

// Initialize Gaussian splat system
RFGaussianSplat rf_splat;
RFGaussianSplat::RFSplatConfig config;
config.num_gaussians = 10000;
config.learning_rate = 0.01;
config.adaptive_density_control = true;
config.densify_grad_threshold = 0.0002;
config.max_iterations = 5000;

rf_splat.initialize(config);

// Load radiance map from instant-rm
RFGaussianSplat::NeuralRadianceMap radiance_map;
radiance_map.path_loss_map = /* ... from instant-rm */;
radiance_map.rms_delay_spread = /* ... */;
radiance_map.mean_direction_arrival = /* ... */;
radiance_map.mean_direction_departure = /* ... */;
radiance_map.measurement_plane_center = Vector3r(0, 0, 1);
radiance_map.measurement_plane_size = Vector2r(100, 100);
radiance_map.num_cells_x = 256;
radiance_map.num_cells_y = 256;
radiance_map.frequency_hz = 2.4e9;
radiance_map.tx_power_dbm = 20.0;

// Train Gaussian representation from radiance map
rf_splat.trainFromRadianceMap(radiance_map);

// Query signal strength at any 3D position
Vector3r query_pos(10.0, 15.0, 1.5);
real_T signal_strength = rf_splat.querySignalStrength(query_pos);
std::cout << "Signal strength: " << signal_strength << " dBm" << std::endl;

// Query gradient (direction of steepest increase)
Vector3r gradient = rf_splat.queryGradient(query_pos);
std::cout << "Gradient: " << gradient.transpose() << " dB/m" << std::endl;
```

### 3. Real-Time Rendering

```cpp
// Render RF field to image
uint32_t width = 1920;
uint32_t height = 1080;
std::vector<float> framebuffer(width * height * 4);

Vector3r camera_pos(50, 50, 20);
Quaternionr camera_rot = Quaternionr::Identity();

rf_splat.renderToImage(
    framebuffer.data(),
    width, height,
    camera_pos, camera_rot
);

// Export to point cloud
std::vector<Vector3r> positions;
std::vector<real_T> signal_strengths;
rf_splat.renderToPointCloud(positions, signal_strengths);
```

### 4. Optimization for Coverage

```cpp
// Optimize transmitter placement for coverage
auto optimize_transmitter = [&](const Vector3r& tx_pos) -> real_T {
    // Generate new radiance map with transmitter at tx_pos
    auto new_radiance_map = generate_instant_rm_map(tx_pos);
    
    // Update Gaussian representation
    rf_splat.setRadianceMap(new_radiance_map);
    rf_splat.trainFromRadianceMap(new_radiance_map);
    
    // Compute coverage metric
    std::vector<real_T> coverage = rf_splat.queryCoverageArea(-80.0, 1.0);
    return coverage.size();  // Number of points with signal > -80 dBm
};

// Gradient-based optimization
Vector3r best_tx_pos = gradient_descent(optimize_transmitter, initial_tx_pos);
```

## Integration with instant-rm

### Option 1: Python Embedding (Recommended)

Embed Python interpreter in C++ to call instant-rm directly.

**Advantages**:
- Full instant-rm functionality
- No need to port algorithms
- Easy to update with upstream changes

**Disadvantages**:
- Python runtime dependency
- Complex data marshaling between C++/Python
- Potential performance overhead

**Implementation**:

```cpp
#include <Python.h>
#include <numpy/arrayobject.h>

class InstantRMBridge {
public:
    bool initialize() {
        Py_Initialize();
        import_array();
        
        // Import instant_rm module
        instant_rm_module_ = PyImport_ImportModule("instant_rm");
        if (!instant_rm_module_) {
            PyErr_Print();
            return false;
        }
        
        return true;
    }
    
    NeuralRadianceMap computeRadianceMap(
        const Vector3r& tx_position,
        real_T tx_power_dbm,
        real_T frequency_hz,
        const std::string& scene_file)
    {
        // Call instant-rm Python API
        PyObject* tracer_class = PyObject_GetAttrString(instant_rm_module_, "MapTracer");
        PyObject* tracer = PyObject_CallObject(tracer_class, /* args */);
        
        // Convert C++ vectors to NumPy arrays
        npy_intp dims[3] = {256, 256, 1};
        PyObject* tx_pos_array = PyArray_SimpleNewFromData(1, dims, NPY_DOUBLE, 
                                                           (void*)tx_position.data());
        
        // Call tracer
        PyObject* result = PyObject_CallMethod(tracer, "__call__", /* args */);
        
        // Convert NumPy arrays back to C++ vectors
        NeuralRadianceMap radiance_map;
        PyObject* path_loss = PyDict_GetItemString(result, "path_loss_map");
        // ... extract data ...
        
        return radiance_map;
    }
    
private:
    PyObject* instant_rm_module_;
};
```

### Option 2: Port Core Algorithms

Port essential instant-rm algorithms to C++.

**Advantages**:
- No Python dependency
- Better performance (no Python/C++ boundary)
- Full control over implementation

**Disadvantages**:
- Large porting effort
- Need to maintain parity with upstream
- May not support all features

**Key Algorithms to Port**:

1. **Ray Tube Tracing**:
```cpp
class RayTubeTracer {
    struct RayTube {
        Vector3r origin, direction;
        real_T field_strength;
        real_T rho_1, rho_2;  // Radii of curvature
        real_T solid_angle;
    };
    
    std::vector<RayTube> traceRays(
        const Vector3r& tx_position,
        const Vector3r& tx_direction,
        uint32_t num_samples)
    {
        std::vector<RayTube> rays;
        
        // Fibonacci lattice sampling on hemisphere
        for (uint32_t i = 0; i < num_samples; ++i) {
            real_T phi = 2.0 * PI * i / GOLDEN_RATIO;
            real_T cos_theta = 1.0 - (i + 0.5) / num_samples;
            real_T sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
            
            Vector3r direction(
                std::cos(phi) * sin_theta,
                std::sin(phi) * sin_theta,
                cos_theta
            );
            
            RayTube ray;
            ray.origin = tx_position;
            ray.direction = direction;
            ray.field_strength = 1.0;  // Normalized
            ray.rho_1 = 0.0;
            ray.rho_2 = 0.0;
            ray.solid_angle = 4.0 * PI / num_samples;
            
            rays.push_back(ray);
        }
        
        return rays;
    }
};
```

2. **Mueller Matrix Propagation**:
```cpp
class MuellerMatrixPropagator {
    using Matrix4r = Eigen::Matrix<real_T, 4, 4>;
    
    Matrix4r computeReflectionMatrix(
        const Vector3r& incident,
        const Vector3r& normal,
        real_T epsilon_r)  // Relative permittivity
    {
        // Compute Fresnel coefficients
        real_T cos_i = -incident.dot(normal);
        real_T sin_i = std::sqrt(1.0 - cos_i * cos_i);
        real_T sin_t = sin_i / std::sqrt(epsilon_r);
        real_T cos_t = std::sqrt(1.0 - sin_t * sin_t);
        
        real_T r_parallel = (epsilon_r * cos_i - cos_t) / (epsilon_r * cos_i + cos_t);
        real_T r_perp = (cos_i - epsilon_r * cos_t) / (cos_i + epsilon_r * cos_t);
        
        // Build Mueller matrix for reflection
        Matrix4r M;
        M << 0.5 * (r_parallel * r_parallel + r_perp * r_perp), /* ... */;
        
        return M;
    }
};
```

3. **Measurement Plane Intersection**:
```cpp
bool intersectMeasurementPlane(
    const RayTube& ray,
    const Vector3r& plane_center,
    const Vector3r& plane_normal,
    const Vector2r& plane_size,
    Vector2r& cell_coords)
{
    // Ray-plane intersection
    real_T denom = ray.direction.dot(plane_normal);
    if (std::abs(denom) < 1e-6) return false;
    
    Vector3r to_plane = plane_center - ray.origin;
    real_T t = to_plane.dot(plane_normal) / denom;
    
    if (t < 0) return false;
    
    // Intersection point
    Vector3r hit_point = ray.origin + ray.direction * t;
    
    // Convert to plane-local coordinates
    Vector3r local_hit = hit_point - plane_center;
    cell_coords.x() = local_hit.x() / plane_size.x() + 0.5;
    cell_coords.y() = local_hit.y() / plane_size.y() + 0.5;
    
    return cell_coords.x() >= 0 && cell_coords.x() <= 1 &&
           cell_coords.y() >= 0 && cell_coords.y() <= 1;
}
```

### Option 3: IPC Service

Run instant-rm as a separate Python service, communicate via gRPC or ZeroMQ.

**Advantages**:
- Language isolation
- Scalability (multiple clients)
- Easy debugging

**Disadvantages**:
- Network latency
- Deployment complexity
- Serialization overhead

## Performance Optimization

### Adaptive Density Control

Automatically refine Gaussian representation during training:

```cpp
// Split high-gradient Gaussians
void RFGaussianSplat::densifyGaussians() {
    for (uint32_t g = 0; g < gaussians_.size(); ++g) {
        real_T grad_magnitude = position_gradients_[g].norm();
        
        if (grad_magnitude > config_.densify_grad_threshold) {
            // Split into two smaller Gaussians
            Gaussian3D split1 = gaussians_[g];
            Gaussian3D split2 = gaussians_[g];
            
            split1.scale *= 0.8;
            split2.scale *= 0.8;
            
            Vector3r offset = position_gradients_[g].normalized() * 0.1;
            split1.position += offset;
            split2.position -= offset;
            
            addGaussian(split1);
            addGaussian(split2);
            removeGaussian(g);
        }
    }
}

// Prune low-opacity Gaussians
void RFGaussianSplat::pruneGaussians() {
    gaussians_.erase(
        std::remove_if(gaussians_.begin(), gaussians_.end(),
            [](const Gaussian3D& g) { return g.opacity < 0.05; }),
        gaussians_.end()
    );
}
```

### Spherical Harmonics

Use spherical harmonics for view-dependent RF effects (multipath, directivity):

```cpp
// Degree L spherical harmonics: (L+1)^2 coefficients
struct SHCoefficients {
    std::vector<Vector3r> coeffs;  // RGB per coefficient
};

real_T evaluateSH(const SHCoefficients& sh, const Vector3r& direction, uint32_t degree) {
    real_T result = 0.0;
    
    // Evaluate spherical harmonic basis functions
    for (uint32_t l = 0; l <= degree; ++l) {
        for (int m = -static_cast<int>(l); m <= static_cast<int>(l); ++m) {
            real_T Y_lm = sphericalHarmonicBasis(l, m, direction);
            uint32_t idx = l * l + l + m;
            result += sh.coeffs[idx].x() * Y_lm;
        }
    }
    
    return result;
}
```

## Use Cases

### 1. Autonomous Swarm Communication

Optimize UAV positions for maximum communication coverage:

```cpp
#include "ai/AgenticSwarmController.hpp"
#include "communication/RFGaussianSplat.hpp"

AgenticSwarmController swarm;
RFGaussianSplat rf_splat;

// Generate radiance map for current swarm configuration
auto radiance_map = generate_instant_rm_map(swarm.getVehiclePositions());
rf_splat.trainFromRadianceMap(radiance_map);

// Optimize swarm positions for coverage
for (uint32_t iteration = 0; iteration < 100; ++iteration) {
    for (auto& vehicle : swarm.getVehicles()) {
        // Compute gradient of coverage w.r.t. vehicle position
        Vector3r grad = rf_splat.queryGradient(vehicle.position);
        
        // Move vehicle in direction of better coverage
        vehicle.target_position = vehicle.position + grad.normalized() * 1.0;
    }
    
    swarm.update(0.1);  // 100 ms timestep
    
    // Re-generate radiance map
    radiance_map = generate_instant_rm_map(swarm.getVehiclePositions());
    rf_splat.trainFromRadianceMap(radiance_map);
}
```

### 2. Real-Time RF Visualization

Render RF fields in real-time for operator situational awareness:

```cpp
// In main update loop
void update(real_T dt) {
    // Update Gaussian representation from live measurements
    std::vector<RFGaussianSplat::TrainingPoint> measurements = collect_rf_measurements();
    rf_splat.trainFromMeasurements(measurements);
    
    // Render to display
    uint32_t width = 1920;
    uint32_t height = 1080;
    std::vector<float> framebuffer(width * height * 4);
    
    rf_splat.renderToImage(
        framebuffer.data(),
        width, height,
        camera_position,
        camera_rotation
    );
    
    display_image(framebuffer);
}
```

### 3. Coverage Prediction

Predict RF coverage for mission planning:

```cpp
// Plan mission waypoints with guaranteed communication
std::vector<Vector3r> plan_waypoints(
    const Vector3r& start,
    const Vector3r& goal,
    real_T min_signal_dbm)
{
    std::vector<Vector3r> waypoints;
    
    // A* search with RF coverage constraint
    auto cost_function = [&](const Vector3r& pos) -> real_T {
        real_T signal = rf_splat.querySignalStrength(pos);
        
        if (signal < min_signal_dbm) {
            return std::numeric_limits<real_T>::infinity();
        }
        
        return 1.0;  // Unit cost if signal adequate
    };
    
    waypoints = a_star_search(start, goal, cost_function);
    
    return waypoints;
}
```

## Comparison: Analytical vs. Neural RF Propagation

| Feature               | RTXRFPropagation (Analytical)                    | RFGaussianSplat (Neural)     |
| --------------------- | ------------------------------------------------ | ---------------------------- |
| **Method**            | Physics-based models (Friis, Two-Ray, ITM, etc.) | Learned from data/simulation |
| **Speed**             | Fast (analytical formulas)                       | Very fast (cached Gaussians) |
| **Accuracy**          | Model-dependent                                  | Data-dependent               |
| **Memory**            | Low (parameters only)                            | Medium (Gaussian storage)    |
| **Interpolation**     | Limited                                          | Excellent                    |
| **Extrapolation**     | Good (physics-based)                             | Poor (out-of-distribution)   |
| **Differentiability** | Manual derivatives                               | Automatic (gradient descent) |
| **Adaptability**      | Fixed models                                     | Learns from environment      |
| **Use Cases**         | Quick estimates, known environments              | Complex scenes, optimization |

## Dependencies

### Required

- **Eigen3**: Matrix/vector operations
- **C++17**: Standard library features
- **CUDA** (optional): GPU-accelerated rendering

### Optional

- **Python 3.8+**: For instant-rm integration (Option 1)
  - Mitsuba 3 with `cuda_ad_mono_polarized` variant
  - Dr.Jit for automatic differentiation
  - NumPy for array operations
- **gRPC** (Option 3): For IPC service architecture

### Installing instant-rm

```bash
# Install Mitsuba 3 with CUDA variant
pip install mitsuba

# Verify variant
python -c "import mitsuba as mi; mi.set_variant('cuda_ad_mono_polarized'); print('OK')"

# Install instant-rm
git clone https://github.com/NVlabs/instant-rm.git
cd instant-rm
pip install -e .
```

## API Reference

### RFGaussianSplat

Main class for Gaussian splatting RF field representation.

#### Initialization

```cpp
bool initialize(const RFSplatConfig& config);
void shutdown();
```

#### Configuration

```cpp
void setConfig(const RFSplatConfig& config);
RFSplatConfig getConfig() const;
```

#### Neural Radiance Map

```cpp
void setRadianceMap(const NeuralRadianceMap& radiance_map);
NeuralRadianceMap getRadianceMap() const;
bool trainFromRadianceMap(const NeuralRadianceMap& radiance_map);
```

#### Gaussian Management

```cpp
void addGaussian(const Gaussian3D& gaussian);
void removeGaussian(uint32_t index);
void updateGaussian(uint32_t index, const Gaussian3D& gaussian);
Gaussian3D getGaussian(uint32_t index) const;
uint32_t getNumGaussians() const;
std::vector<Gaussian3D> getAllGaussians() const;
```

#### Training

```cpp
bool trainFromMeasurements(const std::vector<TrainingPoint>& training_data);
void resetTraining();
real_T computeLoss() const;
```

#### Query

```cpp
real_T querySignalStrength(const Vector3r& position) const;
Vector3r queryGradient(const Vector3r& position) const;
std::vector<real_T> queryCoverageArea(real_T min_signal_dbm, real_T resolution_m) const;
```

#### Rendering

```cpp
void renderToImage(void* image_buffer, uint32_t width, uint32_t height,
                  const Vector3r& camera_position, const Quaternionr& camera_rotation) const;
void renderToPointCloud(std::vector<Vector3r>& positions, 
                       std::vector<real_T>& signal_strengths) const;
```

#### Optimization

```cpp
void optimizeStep();
void optimizeMultiStep(uint32_t num_steps);
void densifyGaussians();
void pruneGaussians();
void resetOpacity();
```

#### Import/Export

```cpp
bool exportToFile(const std::string& filename) const;
bool importFromFile(const std::string& filename);
```

#### Statistics

```cpp
real_T getAverageSignalStrength() const;
real_T getMinSignalStrength() const;
real_T getMaxSignalStrength() const;
uint32_t getTrainingIteration() const;
real_T getCurrentLoss() const;
```

## Future Enhancements

1. **Temporal Gaussians**: Time-varying RF fields for dynamic environments
2. **Multi-Frequency Gaussians**: Frequency-dependent splats for wideband signals
3. **Uncertainty Quantification**: Gaussian covariance as prediction uncertainty
4. **Active Learning**: Select optimal measurement locations
5. **LoD (Level of Detail)**: Adaptive Gaussian density based on view distance
6. **Multi-Resolution Hierarchies**: Octree-based Gaussian organization
7. **Neural Shader Integration**: Direct integration with Unreal Engine materials

## References

1. NVIDIA instant-rm: https://github.com/NVlabs/instant-rm
2. 3D Gaussian Splatting: https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/
3. Mitsuba 3: https://mitsuba-renderer.org
4. Dr.Jit: https://github.com/mitsuba-renderer/drjit

## License

This integration follows the licenses of the underlying components:
- instant-rm: NVIDIA Source Code License
- 3D Gaussian Splatting: Inria License
- Mitsuba 3: BSD-3-Clause License

## Support

For issues or questions:
- AutonomySim GitHub: https://github.com/nervosys/AutonomySim
- instant-rm GitHub: https://github.com/NVlabs/instant-rm

