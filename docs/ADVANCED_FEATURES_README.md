# Advanced Unmanned Vehicle Features - Quick Start

## Overview
This implementation adds two major subsystems to AutonomySim:

1. **PropulsionModel** - Physics-based propulsion for multiple vehicle types
2. **FormationControl** - Multi-vehicle formation control and coordination

## Quick Start

### 1. Propulsion Model Example

```cpp
#include "physics/PropulsionModel.hpp"

// Create a quadcopter rotor model
PropulsionModel::PropulsionParams params;
params.type = PropulsionModel::PropulsionType::Rotor;
params.diameter = 0.254f;  // 10" rotor
params.max_rpm = 10000.0f;

PropulsionModel rotor(params);

// Compute thrust at 75% throttle
auto output = rotor.compute(0.75f, 0.0f);
std::cout << "Thrust: " << output.thrust << " N\n";
```

### 2. Formation Control Example

```cpp
#include "control/FormationControl.hpp"

// Setup wedge formation
FormationControl::FormationParams params;
params.type = FormationControl::FormationType::Wedge;
params.spacing = 5.0f;

FormationControl formation(params);

// Get desired position for vehicle in formation
Vector3r desired_pos = formation.getDesiredPosition(
    vehicle_id, leader_state, num_vehicles);
```

## Files Added

### Headers
- `AutonomyLib/include/physics/PropulsionModel.hpp`
- `AutonomyLib/include/control/FormationControl.hpp`

### Implementation
- `AutonomyLib/src/physics/PropulsionModel.cpp`
- `AutonomyLib/src/control/FormationControl.cpp`

### Examples
- `Examples/PropulsionFormationExample.cpp`

### Documentation
- `docs/advanced_unmanned_features.md`

## Features Implemented

### PropulsionModel
- ✅ 6 propulsion types: Propeller, Rotor, Thruster, Wheel, Track, Jet
- ✅ Physics-based thrust/torque computation
- ✅ Wageningen B-series for marine propellers
- ✅ Momentum theory for UAV rotors
- ✅ Ducted thruster effects for UUVs
- ✅ Wheel/track models for UGVs
- ✅ Advance ratio and efficiency calculations
- ✅ Multi-domain support (air, water, ground)

### FormationControl
- ✅ 7 formation types: Line, Column, Wedge, Diamond, Circle, Box, Custom
- ✅ Collision avoidance (inverse-square repulsion)
- ✅ Cohesion and alignment forces
- ✅ Leader-follower control
- ✅ Potential field approach
- ✅ Configurable control gains
- ✅ Custom formation definitions

## Building

The new files are integrated into the existing AutonomySim build system:

```powershell
# Windows
cd AutonomySim
.\scripts\build.ps1

# Linux
cd AutonomySim
./scripts/build.sh
```

## Running the Example

After building:

```powershell
.\build\Examples\PropulsionFormationExample.exe
```

This demonstrates:
- Propulsion calculations for different vehicle types
- Formation geometries
- Control commands with collision avoidance

## Integration with Existing Code

### MultiRotor Integration

```cpp
// In MultiRotorPhysicsBody or similar class
PropulsionModel rotor_models[4];  // For quadcopter

// Initialize
for (int i = 0; i < 4; ++i) {
    PropulsionModel::PropulsionParams params;
    params.type = PropulsionModel::PropulsionType::Rotor;
    params.diameter = rotor_diameter;
    params.max_rpm = max_rotor_rpm;
    rotor_models[i].initialize(params);
}

// Update physics
for (int i = 0; i < 4; ++i) {
    auto output = rotor_models[i].compute(
        rotor_commands[i], 
        getVelocity().norm());
    applyRotorForce(i, output.thrust, output.torque);
}
```

### Car Integration

```cpp
// In CarPhysicsBody or similar
PropulsionModel wheel_models[4];

// Initialize
for (int i = 0; i < 4; ++i) {
    PropulsionModel::PropulsionParams params;
    params.type = PropulsionModel::PropulsionType::Wheel;
    params.diameter = wheel_radius;
    params.gear_ratio = transmission_ratio;
    wheel_models[i].initialize(params);
}

// Update physics
for (int i = 0; i < 4; ++i) {
    auto output = wheel_models[i].compute(
        throttle,
        0.0f,
        wheel_angular_velocity[i]);
    applyWheelForce(i, output.thrust);
}
```

### Multi-Vehicle Formation

```cpp
// In WorldSimApi or custom controller
FormationControl formation_controller;
std::vector<FormationControl::VehicleState> swarm_states;

// Update loop
for (int i = 1; i < num_vehicles; ++i) {
    auto command = formation_controller.computeCommand(
        i, 
        swarm_states[i], 
        swarm_states, 
        swarm_states[0]);  // Leader
    
    vehicles[i].setDesiredVelocity(command.desired_velocity);
}
```

## Mathematical Background

### Propulsion Physics

**Marine Propeller (B-Series)**:
```
T = Kt × ρ × n² × D⁴
Q = Kq × ρ × n² × D⁵
J = V / (n × D)
```

**UAV Rotor (Momentum Theory)**:
```
P_induced = T^(3/2) / √(2ρA)
```

**UGV Wheel**:
```
F_traction = τ / r
```

### Formation Control

**Control Law**:
```
v_cmd = K_p(p_d - p) + K_v(v_d - v) + K_s·F_sep + K_c·F_coh + K_a·F_align
```

**Collision Avoidance**:
```
F_separation = Σ(1/d²) · n̂
```

## Configuration Parameters

### Typical UAV Rotor
```
Diameter: 0.20 - 0.30 m (8-12 inches)
Max RPM: 8,000 - 12,000
Efficiency: 0.80 - 0.90
Air Density: 1.225 kg/m³ (sea level)
```

### Typical USV Propeller
```
Diameter: 0.15 - 0.40 m
Pitch: 0.10 - 0.30 m
Max RPM: 1,500 - 4,000
Efficiency: 0.65 - 0.80
Water Density: 1000 kg/m³ (fresh) or 1025 kg/m³ (salt)
```

### Typical Formation Parameters
```
Spacing: 3-10 m (vehicle dependent)
Collision Radius: 1-3 m
K_position: 0.5-2.0
K_separation: 1.0-3.0
K_cohesion: 0.1-0.5
K_alignment: 0.1-0.3
```

## Testing

Run the example to verify the implementation:

```powershell
.\build\Examples\PropulsionFormationExample.exe
```

Expected output includes:
- Thrust/power calculations for different propulsion types
- Formation positions for different geometries
- Control commands showing collision avoidance

## Performance

- **PropulsionModel**: O(1), can run at 1000+ Hz
- **FormationControl**: O(n) per vehicle, typically 10-50 Hz sufficient
- **Memory**: Minimal overhead (~200 bytes per PropulsionModel, ~300 bytes per FormationControl)

## Future Enhancements

Potential additions (not yet implemented):
- [ ] Adaptive formation reconfiguration
- [ ] Obstacle avoidance in formations
- [ ] Dynamic formation type switching
- [ ] Wind/current disturbance modeling in propulsion
- [ ] Battery consumption estimation
- [ ] Multi-agent path planning integration

## Troubleshooting

### Build Issues
- Ensure all source files are in the correct directories
- Check that AutonomyLib.vcxproj includes the new files
- Verify C++14 or later compiler support

### Runtime Issues
- Check that PropulsionParams are properly initialized
- Verify fluid density matches vehicle environment (air vs water)
- Ensure formation spacing is appropriate for vehicle size

## License

Copyright (c) nervosys. All rights reserved.
Licensed under the MIT License.

## Contact

For questions or issues, please refer to the main AutonomySim documentation.
