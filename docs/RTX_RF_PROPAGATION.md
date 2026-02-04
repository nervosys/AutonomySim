# RTX RF Propagation Module

## Overview

The RTX RF Propagation module provides real-time radio frequency (RF) signal propagation simulation for autonomous vehicle swarms using NVIDIA RTX hardware-accelerated ray tracing. This enables physically accurate communication link modeling, taking into account:

- **3D Environment Geometry**: Buildings, terrain, vegetation obstacles
- **Material Properties**: Concrete, metal, wood, glass attenuation
- **Weather Effects**: Rain, fog, snow impact on signal strength
- **Multi-Frequency Support**: VHF (30 MHz) to EHF (300 GHz)
- **Real-Time Performance**: RTX cores for fast ray tracing

## Features

### Core Capabilities

1. **Multiple Propagation Models**:
   - Free Space (Friis equation)
   - Two-Ray Ground Reflection
   - Okumura-Hata (Urban/Suburban)
   - COST231-Hata
   - Walfish-Ikegami
   - **RTX Ray Tracing** (Hardware-accelerated)

2. **Environmental Effects**:
   - Weather conditions (clear, rain, fog, snow, storm)
   - Atmospheric attenuation
   - Ground reflection
   - Material-specific penetration loss

3. **RF Analysis**:
   - Line-of-sight determination
   - Path loss calculation
   - Received signal strength (RSSI)
   - Signal-to-noise ratio (SNR)
   - Signal-to-interference-plus-noise ratio (SINR)
   - Fresnel zone clearance
   - Doppler shift for moving vehicles

4. **Advanced Features**:
   - Coverage area analysis
   - Interference simulation
   - Jamming scenarios
   - Multipath reflections
   - Diffraction modeling

## Hardware Requirements

### Minimum
- **GPU**: NVIDIA RTX 2060 or newer
- **VRAM**: 8 GB
- **Driver**: 470.x or newer
- **OptiX**: Version 7.x or 8.x

### Recommended
- **GPU**: NVIDIA RTX 3070 or newer
- **VRAM**: 12 GB+
- **Driver**: Latest GeForce/Quadro driver

### Optimal
- **GPU**: NVIDIA RTX 4090
- **VRAM**: 24 GB
- **CPU**: Multi-core for parallel query processing

## Installation

### 1. Install NVIDIA OptiX SDK

```powershell
# Download from NVIDIA Developer Portal
https://developer.nvidia.com/designworks/optix/download

# Install to default location
C:\ProgramData\NVIDIA Corporation\OptiX SDK 8.0.0\
```

### 2. Add OptiX to Project

Update `AutonomyLib.vcxproj` with OptiX include/lib paths:

```xml
<AdditionalIncludeDirectories>
    $(OPTIX_PATH)\include;
    %(AdditionalIncludeDirectories)
</AdditionalIncludeDirectories>

<AdditionalLibraryDirectories>
    $(OPTIX_PATH)\lib64;
    %(AdditionalLibraryDirectories)
</AdditionalLibraryDirectories>
```

### 3. Build AutonomyLib

```powershell
cd AutonomySim
msbuild AutonomySim.sln /p:Configuration=Release /p:Platform=x64
```

## Quick Start

### Basic Usage

```cpp
#include "communication/RTXRFPropagation.hpp"

using namespace autonomysim;

// Initialize RF propagation
RTXRFPropagation rf;
RTXRFPropagation::RTXConfig config;
config.enable_rtx = true;
config.model = RTXRFPropagation::PropagationModel::RTX_RayTracing;
config.weather = RTXRFPropagation::WeatherCondition::Clear;
config.max_ray_bounces = 3;
config.rays_per_query = 32;

rf.initialize(config);

// Register transmitter (drone)
RTXRFPropagation::Transmitter tx;
tx.id = "drone1_tx";
tx.position = Vector3r(0, 0, 50);  // 50m altitude
tx.frequency_hz = 2.4e9;           // 2.4 GHz
tx.power_dbm = 20.0;               // 20 dBm (100 mW)
tx.antenna_gain_dbi = 2.0;
rf.registerTransmitter(tx);

// Register receiver (ground station)
RTXRFPropagation::Receiver rx;
rx.id = "gcs_rx";
rx.position = Vector3r(1000, 0, 0);  // 1 km away
rx.sensitivity_dbm = -90.0;
rx.antenna_gain_dbi = 6.0;
rf.registerReceiver(rx);

// Compute propagation
RTXRFPropagation::PropagationResult result = rf.computePropagation("drone1_tx", "gcs_rx");

std::cout << "Distance: " << result.distance_m << " m\n";
std::cout << "Path Loss: " << result.path_loss_db << " dB\n";
std::cout << "RSSI: " << result.received_power_dbm << " dBm\n";
std::cout << "SNR: " << result.signal_to_noise_db << " dB\n";
std::cout << "LOS: " << (result.line_of_sight ? "Yes" : "No") << "\n";
```

## API Reference

### Configuration

#### RTXConfig

```cpp
struct RTXConfig {
    bool enable_rtx;                    // Use hardware ray tracing
    uint32_t max_ray_bounces;           // Max reflections (1-10)
    uint32_t rays_per_query;            // Rays per calculation (trade quality vs speed)
    bool enable_atmospheric;            // Atmospheric loss
    bool enable_ground_reflection;      // Ground bounce
    bool enable_diffraction;            // Diffraction around edges
    PropagationModel model;             // Propagation model to use
    WeatherCondition weather;           // Current weather
};
```

#### PropagationModel

```cpp
enum class PropagationModel {
    FreeSpace,           // Friis equation (ideal)
    TwoRay,              // Ground reflection
    Longley_Rice,        // ITU-R irregular terrain
    Okumura_Hata,        // Urban empirical (150-1500 MHz)
    COST231_Hata,        // Extended Okumura-Hata (1500-2000 MHz)
    Walfish_Ikegami,     // Urban street canyon
    RTX_RayTracing       // Hardware-accelerated (most accurate)
};
```

#### WeatherCondition

```cpp
enum class WeatherCondition {
    Clear,               // No weather impact
    LightRain,           // ~5 mm/hr
    ModerateRain,        // ~25 mm/hr
    HeavyRain,           // ~50 mm/hr
    Fog,                 // Dense fog (visibility < 50m)
    Snow,                // Snowfall
    Storm                // Severe weather
};
```

### RF Bands

```cpp
enum class RFBand {
    VHF_30_300MHz,       // VHF (TV, FM radio)
    UHF_300MHz_3GHz,     // UHF (WiFi 2.4 GHz, cellular)
    SHF_3_30GHz,         // SHF (WiFi 5 GHz, radar)
    EHF_30_300GHz,       // EHF (mmWave 5G)
    Custom               // User-defined frequency
};
```

### Material Properties

```cpp
enum class MaterialType {
    Air,                 // Free space
    Concrete,            // ~15 dB/m attenuation
    Metal,               // ~100 dB/m (near total blocking)
    Wood,                // ~5 dB/m
    Brick,               // ~10 dB/m
    Glass,               // ~2 dB/m
    Vegetation,          // ~0.5 dB/m (frequency dependent)
    Water,               // ~25 dB/m
    Soil,                // ~12 dB/m
    Asphalt,             // ~8 dB/m
    Custom               // User-defined properties
};

struct MaterialProperties {
    MaterialType type;
    real_T relative_permittivity;      // Dielectric constant
    real_T conductivity_s_m;            // Electrical conductivity (S/m)
    real_T attenuation_db_per_m;        // Attenuation coefficient
    real_T reflection_coefficient;      // Surface reflection (0-1)
    real_T roughness_m;                 // Surface roughness
};
```

### Transmitter & Receiver

```cpp
struct Transmitter {
    std::string id;
    Vector3r position;
    real_T frequency_hz;
    real_T power_dbm;
    real_T antenna_gain_dbi;
    real_T beamwidth_degrees;
    real_T antenna_height_m;
    RFBand band;
    bool active;
};

struct Receiver {
    std::string id;
    Vector3r position;
    real_T sensitivity_dbm;
    real_T antenna_gain_dbi;
    real_T noise_figure_db;
    bool active;
};
```

### Propagation Result

```cpp
struct PropagationResult {
    real_T distance_m;
    real_T path_loss_db;
    real_T received_power_dbm;
    real_T signal_to_noise_db;
    bool line_of_sight;
    real_T fresnel_clearance;          // 1.0 = 100% clear
    uint32_t num_reflections;
    uint32_t num_diffractions;
    real_T delay_spread_ns;            // For multipath
    std::vector<Vector3r> ray_path;    // For visualization
    std::string status;
};
```

### Core Methods

```cpp
bool initialize(const RTXConfig& config);
void shutdown();

void registerTransmitter(const Transmitter& tx);
void registerReceiver(const Receiver& rx);

PropagationResult computePropagation(const std::string& tx_id, const std::string& rx_id);
std::vector<PropagationResult> computeAllLinks();

bool hasLineOfSight(const Vector3r& tx_pos, const Vector3r& rx_pos) const;
real_T estimatePathLoss(const Vector3r& tx_pos, const Vector3r& rx_pos, real_T frequency_hz);

void updateTransmitter(const std::string& tx_id, const Vector3r& position);
void updateReceiver(const std::string& rx_id, const Vector3r& position);
```

### Advanced Methods

```cpp
// Coverage analysis
std::vector<Vector3r> computeCoverageArea(const std::string& tx_id, 
                                          real_T min_rssi_dbm, 
                                          real_T grid_resolution_m);

// Interference simulation
void registerJammer(const Transmitter& jammer);
real_T computeInterference(const Receiver& rx, real_T signal_frequency_hz);
real_T computeSINR(const std::string& tx_id, const std::string& rx_id);

// Doppler shift
void setTransmitterVelocity(const std::string& tx_id, const Vector3r& velocity_m_s);
void setReceiverVelocity(const std::string& rx_id, const Vector3r& velocity_m_s);
real_T computeDopplerShift(const Transmitter& tx, const Receiver& rx,
                           const Vector3r& tx_velocity, const Vector3r& rx_velocity);

// Scene integration
void setScene(void* unreal_scene_ptr);
void setTerrainElevation(const std::vector<std::vector<real_T>>& elevation_grid_m);

// Statistics
real_T getAverageComputeTimeMs() const;
uint32_t getRTXCoreUtilization() const;
```

## Examples

### Example 1: Drone-to-Drone Communication

```cpp
// Urban drone swarm scenario
RTXRFPropagation rf;
RTXConfig config;
config.enable_rtx = true;
config.model = PropagationModel::RTX_RayTracing;
config.weather = WeatherCondition::Clear;
rf.initialize(config);

// Drone 1 (transmitter)
Transmitter drone1;
drone1.id = "drone1_tx";
drone1.position = Vector3r(0, 0, 50);
drone1.frequency_hz = 2.4e9;
drone1.power_dbm = 20.0;
drone1.antenna_gain_dbi = 2.0;
rf.registerTransmitter(drone1);

// Drone 2 (receiver)
Receiver drone2;
drone2.id = "drone2_rx";
drone2.position = Vector3r(100, 0, 50);
drone2.sensitivity_dbm = -90.0;
drone2.antenna_gain_dbi = 2.0;
rf.registerReceiver(drone2);

// Check communication
PropagationResult result = rf.computePropagation("drone1_tx", "drone2_rx");
if (result.signal_to_noise_db > 10.0) {
    std::cout << "Link viable: SNR = " << result.signal_to_noise_db << " dB\n";
}
```

### Example 2: Coverage Analysis

```cpp
// Compute coverage area for base station
Transmitter base_station;
base_station.id = "base_tx";
base_station.position = Vector3r(0, 0, 100);  // 100m tower
base_station.frequency_hz = 915e6;            // 915 MHz
base_station.power_dbm = 30.0;                // 30 dBm (1W)
base_station.antenna_gain_dbi = 6.0;
rf.registerTransmitter(base_station);

// Compute coverage at ground level
real_T min_rssi = -90.0;  // Minimum signal strength
real_T resolution = 10.0;  // 10m grid
std::vector<Vector3r> coverage = rf.computeCoverageArea("base_tx", min_rssi, resolution);

std::cout << "Coverage area: " << coverage.size() << " points\n";
```

### Example 3: Jamming Scenario

```cpp
// Add hostile jammer
Transmitter jammer;
jammer.id = "jammer1";
jammer.position = Vector3r(50, 50, 0);
jammer.frequency_hz = 2.4e9;
jammer.power_dbm = 40.0;  // 40 dBm (10W)
jammer.antenna_gain_dbi = 6.0;
rf.registerJammer(jammer);

// Compute SINR (signal-to-interference-plus-noise ratio)
real_T sinr = rf.computeSINR("drone1_tx", "drone2_rx");
if (sinr < 0.0) {
    std::cout << "Link jammed: SINR = " << sinr << " dB\n";
}
```

## Performance Optimization

### Tips for Real-Time Operation

1. **Adjust Ray Count**:
   ```cpp
   config.rays_per_query = 16;  // Faster, less accurate
   config.rays_per_query = 64;  // Slower, more accurate
   ```

2. **Limit Reflections**:
   ```cpp
   config.max_ray_bounces = 2;  // Urban environment
   config.max_ray_bounces = 5;  // Complex indoor
   ```

3. **Use Appropriate Model**:
   - **Free Space**: Open sky (fastest)
   - **Two-Ray**: Over water/ground
   - **Okumura-Hata**: Urban (fast empirical)
   - **RTX Ray Tracing**: Complex 3D (most accurate)

4. **Batch Queries**:
   ```cpp
   std::vector<PropagationResult> all_links = rf.computeAllLinks();
   ```

### Performance Benchmarks

**NVIDIA RTX 3080 (10GB)**:
- Free Space: 0.05 ms/query
- Two-Ray: 0.1 ms/query
- Okumura-Hata: 0.15 ms/query
- RTX Ray Tracing: 2-5 ms/query (depends on scene complexity)

**Target Performance**:
- 10 drones: >30 Hz update rate
- 50 drones: >10 Hz update rate
- 100 drones: >1 Hz update rate

## Integration with Swarm AI

The RTX RF Propagation module integrates seamlessly with AutonomySim's agentic AI:

```cpp
#include "communication/RTXRFPropagation.hpp"
#include "ai/AgenticSwarmController.hpp"
#include "ai/MCPServer.hpp"
#include "ai/A2AProtocol.hpp"

// Use RF propagation for network-aware task allocation
AgenticSwarmController swarm;
RTXRFPropagation rf;

// Check if agents can communicate before assigning collaborative task
if (rf.canCommunicate("drone1", "drone2")) {
    swarm.assignCollaborativeTask("drone1", "drone2", task);
}

// Adapt formation based on link quality
real_T min_rssi = -80.0;  // Required signal strength
swarm.optimizeFormationForCommunication(min_rssi);
```

## Troubleshooting

### RTX Not Available

```
[WARN] RTX hardware not available, falling back to analytical models
```

**Solution**: Verify NVIDIA RTX GPU installed and drivers updated.

### Low Performance

```cpp
// Check RTX utilization
uint32_t utilization = rf.getRTXCoreUtilization();
if (utilization < 50) {
    // Increase workload or reduce ray count
}
```

### Inaccurate Results

1. Verify material properties match scene
2. Check weather conditions
3. Increase ray count for complex scenes
4. Ensure scene geometry is updated

## References

1. **Friis Transmission Equation**: IEEE Standard 145-2013
2. **Okumura-Hata Model**: ITU-R P.1546
3. **NVIDIA OptiX**: https://developer.nvidia.com/optix
4. **RF Propagation**: "Wireless Communications" by Rappaport (2002)

## License

Part of AutonomySim - MIT License

Copyright (c) 2025 Nervosys

## Support

For issues and questions:
- GitHub: https://github.com/nervosys/AutonomySim
- Documentation: See `docs/RTX_RF_PROPAGATION.md`
