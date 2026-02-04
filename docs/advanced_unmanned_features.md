# Advanced Unmanned Vehicle Features

This document describes the advanced propulsion and formation control systems added to AutonomySim.

## Table of Contents
1. [PropulsionModel System](#propulsionmodel-system)
2. [FormationControl System](#formationcontrol-system)
3. [Integration Examples](#integration-examples)
4. [API Reference](#api-reference)

---

## PropulsionModel System

### Overview
The `PropulsionModel` class provides physics-based propulsion modeling for multiple vehicle types:
- **Marine Vehicles** (USV): Propellers with Wageningen B-series coefficients
- **Unmanned Aerial Vehicles** (UAV): Rotors with momentum theory
- **Unmanned Underwater Vehicles** (UUV): Ducted thrusters
- **Unmanned Ground Vehicles** (UGV): Wheels and tracks
- **Fixed-Wing Aircraft**: Jet engines

### Key Features
- **Physics-Based**: Uses real propulsion equations (momentum theory, B-series, etc.)
- **Multi-Domain**: Supports air, water, and ground vehicles
- **Realistic Performance**: Computes thrust, torque, power, efficiency, and RPM
- **Advance Ratio**: Accounts for vehicle velocity effects on propulsion

### Mathematical Models

#### 1. Marine Propeller (Wageningen B-Series)
```
Thrust:  T = Kt × ρ × n² × D⁴
Torque:  Q = Kq × ρ × n² × D⁵
Advance Ratio: J = V / (n × D)
```
Where:
- `Kt`, `Kq` = Thrust and torque coefficients (from B-series curves)
- `ρ` = Fluid density (kg/m³)
- `n` = Rotational speed (rev/s)
- `D` = Propeller diameter (m)
- `V` = Advance velocity (m/s)

#### 2. UAV Rotor (Momentum Theory)
```
Thrust: T ∝ ω² (simplified)
Induced Power: P_ind = T^(3/2) / √(2ρA)
Total Power: P = P_ind + P_profile + P_climb
```
Where:
- `ω` = Angular velocity (rad/s)
- `A` = Rotor disk area (m²)
- `P_profile` ≈ 15% of induced power (blade drag)

#### 3. UUV Thruster
```
Same as propeller, but with duct factor:
T_ducted = T_open × (1 + 0.3 × duct_ratio)
```
Ducting typically increases thrust by ~30% due to pressure recovery.

#### 4. UGV Wheel/Track
```
Traction Force: F = Torque / radius
Power: P = Torque × ω
```

### Usage Example

```cpp
#include "physics/PropulsionModel.hpp"

using namespace nervosys::autonomylib;

// Create UAV rotor propulsion model
PropulsionModel::PropulsionParams params;
params.type = PropulsionModel::PropulsionType::Rotor;
params.diameter = 0.254f;        // 10" rotor
params.max_rpm = 10000.0f;
params.efficiency = 0.85f;
params.air_density = 1.225f;     // Sea level

PropulsionModel rotor(params);

// Compute output at 75% throttle
real_T throttle = 0.75f;
real_T velocity = 5.0f;  // m/s forward flight
auto output = rotor.compute(throttle, velocity);

std::cout << "Thrust: " << output.thrust << " N\n";
std::cout << "Power: " << output.power << " W\n";
std::cout << "RPM: " << output.rpm << "\n";
std::cout << "Efficiency: " << output.efficiency << "\n";
```

### Configuration for Different Vehicle Types

#### Quadcopter (UAV)
```cpp
PropulsionModel::PropulsionParams quad_params;
quad_params.type = PropulsionModel::PropulsionType::Rotor;
quad_params.diameter = 0.254f;      // 10 inch rotor
quad_params.num_blades = 2;
quad_params.max_rpm = 10000.0f;
quad_params.efficiency = 0.85f;
quad_params.air_density = 1.225f;
```

#### Surface Vehicle (USV)
```cpp
PropulsionModel::PropulsionParams usv_params;
usv_params.type = PropulsionModel::PropulsionType::Propeller;
usv_params.diameter = 0.30f;        // 30 cm propeller
usv_params.pitch = 0.20f;           // 20 cm pitch
usv_params.num_blades = 3;
usv_params.max_rpm = 3000.0f;
usv_params.air_density = 1000.0f;   // Water density
```

#### Underwater Vehicle (UUV)
```cpp
PropulsionModel::PropulsionParams uuv_params;
uuv_params.type = PropulsionModel::PropulsionType::Thruster;
uuv_params.diameter = 0.20f;        // 20 cm duct
uuv_params.duct_ratio = 1.3f;       // 30% duct boost
uuv_params.max_rpm = 2500.0f;
uuv_params.air_density = 1025.0f;   // Seawater density
```

#### Ground Vehicle (UGV)
```cpp
PropulsionModel::PropulsionParams ugv_params;
ugv_params.type = PropulsionModel::PropulsionType::Wheel;
ugv_params.diameter = 0.30f;        // 30 cm wheel radius
ugv_params.gear_ratio = 10.0f;      // 10:1 gearbox
ugv_params.efficiency = 0.90f;
```

---

## FormationControl System

### Overview
The `FormationControl` class implements multi-vehicle formation control with:
- **7 Formation Types**: Line, Column, Wedge, Diamond, Circle, Box, Custom
- **Collision Avoidance**: Inverse-square repulsion force
- **Cohesion & Alignment**: Flocking behaviors
- **Leader-Follower**: Vehicles track leader while maintaining formation

### Control Law
The formation control uses a potential field approach:

```
V_desired = K_p × E_pos + K_v × E_vel + K_s × F_sep + K_c × F_coh + K_a × F_align
```

Where:
- `E_pos` = Position error (desired - current)
- `E_vel` = Velocity error
- `F_sep` = Separation force (collision avoidance)
- `F_coh` = Cohesion force (stay with group)
- `F_align` = Alignment force (match velocities)
- `K_p`, `K_v`, `K_s`, `K_c`, `K_a` = Control gains

#### Collision Avoidance
```
F_sep = Σ (1 / d²) × n̂
```
Where `d` is distance to neighbor, `n̂` is unit vector away from neighbor.

### Formation Geometries

#### 1. Line Formation
```
Vehicles arranged horizontally:
    [2] [1] [0] [3] [4]
```

#### 2. Column Formation
```
Vehicles in a line (behind leader):
[0] → [1] → [2] → [3] → [4]
```

#### 3. Wedge Formation (V-Shape)
```
        [0] (Leader)
      [1]  [2]
    [3]      [4]
```

#### 4. Diamond Formation
```
      [0]
   [3]  [1]
      [2]
```

#### 5. Circle Formation
```
      [0]
  [7]    [1]
[6]        [2]
  [5]    [3]
      [4]
```

#### 6. Box Formation
```
[0] [1] [2]
[3] [4] [5]
[6] [7] [8]
```

#### 7. Custom Formation
User-defined relative positions.

### Usage Example

```cpp
#include "control/FormationControl.hpp"

using namespace nervosys::autonomylib;

// Setup formation parameters
FormationControl::FormationParams params;
params.type = FormationControl::FormationType::Wedge;
params.spacing = 5.0f;              // 5m between vehicles
params.collision_radius = 2.0f;     // Avoid within 2m
params.max_velocity = 10.0f;
params.k_position = 1.0f;
params.k_separation = 2.0f;

FormationControl formation(params);

// Define leader state
FormationControl::VehicleState leader;
leader.position = Vector3r(0, 0, 10);    // 10m altitude
leader.velocity = Vector3r(5, 0, 0);     // 5 m/s forward
leader.orientation = Quaternionr::Identity();
leader.id = 0;

// Define follower state
FormationControl::VehicleState follower;
follower.id = 1;
follower.position = Vector3r(-4, 3, 10);  // Slightly off position
follower.velocity = Vector3r(4.5, 0, 0);
follower.orientation = Quaternionr::Identity();

// Create all states vector
std::vector<FormationControl::VehicleState> all_states = {leader, follower};

// Compute control command for follower
auto command = formation.computeCommand(1, follower, all_states, leader);

// Apply command to vehicle
vehicle.setDesiredVelocity(command.desired_velocity);
vehicle.setDesiredAcceleration(command.desired_acceleration);
```

### Custom Formation

```cpp
// Define custom geometry
std::vector<Vector3r> custom_positions = {
    Vector3r(5, 5, 0),      // Position of vehicle 0
    Vector3r(-5, 5, 0),     // Position of vehicle 1
    Vector3r(-5, -5, 0),    // Position of vehicle 2
    Vector3r(5, -5, 0)      // Position of vehicle 3
};

formation.setCustomFormation(custom_positions);
```

### Parameter Tuning Guide

#### Position Control Gain (`k_position`)
- **Low (0.1-0.5)**: Slower convergence, smoother motion
- **Medium (0.5-1.5)**: Balanced performance
- **High (1.5-3.0)**: Fast convergence, may oscillate

#### Separation Gain (`k_separation`)
- **Low (0.5-1.0)**: Vehicles can get closer
- **Medium (1.0-2.5)**: Moderate collision avoidance
- **High (2.5-5.0)**: Strong repulsion, larger spacing

#### Cohesion Gain (`k_cohesion`)
- Controls how strongly vehicles are pulled toward group center
- Typical: 0.2-0.5

#### Alignment Gain (`k_alignment`)
- Controls how strongly vehicles match neighbor velocities
- Typical: 0.1-0.3

---

## Integration Examples

### Multi-Rotor Swarm with Formation Control

```cpp
// Create 4 quadcopters in wedge formation
std::vector<MultiRotorVehicle> swarm(4);

// Setup propulsion for each
PropulsionModel::PropulsionParams rotor_params;
rotor_params.type = PropulsionModel::PropulsionType::Rotor;
rotor_params.diameter = 0.254f;
rotor_params.max_rpm = 10000.0f;

for (auto& vehicle : swarm) {
    vehicle.setPropulsionModel(rotor_params);
}

// Setup formation control
FormationControl::FormationParams form_params;
form_params.type = FormationControl::FormationType::Wedge;
form_params.spacing = 5.0f;
FormationControl formation(form_params);

// Control loop
while (mission_active) {
    // Get current states
    std::vector<FormationControl::VehicleState> states;
    for (int i = 0; i < 4; ++i) {
        states.push_back(swarm[i].getState());
    }
    
    // Compute formation commands for each follower
    for (int i = 1; i < 4; ++i) {
        auto command = formation.computeCommand(i, states[i], states, states[0]);
        
        // Compute required thrust
        Vector3r thrust_vector = command.desired_acceleration * swarm[i].getMass();
        real_T total_thrust = thrust_vector.norm();
        
        // Compute throttle from propulsion model
        real_T throttle = swarm[i].computeThrottleForThrust(total_thrust);
        
        // Apply to vehicle
        swarm[i].setThrottle(throttle);
        swarm[i].setDesiredOrientation(command.desired_orientation);
    }
    
    // Update physics
    for (auto& vehicle : swarm) {
        vehicle.update(dt);
    }
}
```

### Heterogeneous Fleet (UAV + USV Coordination)

```cpp
// UAV for aerial surveillance
PropulsionModel::PropulsionParams uav_params;
uav_params.type = PropulsionModel::PropulsionType::Rotor;
uav_params.diameter = 0.254f;
uav_params.air_density = 1.225f;

// USV for surface patrol
PropulsionModel::PropulsionParams usv_params;
usv_params.type = PropulsionModel::PropulsionType::Propeller;
usv_params.diameter = 0.30f;
usv_params.air_density = 1000.0f;  // Water

// Custom formation: UAV above USV
std::vector<Vector3r> custom_positions = {
    Vector3r(0, 0, 0),      // USV at surface
    Vector3r(0, 0, 20)      // UAV 20m above
};

FormationControl formation;
formation.setCustomFormation(custom_positions);
```

---

## API Reference

### PropulsionModel Class

#### Constructor
```cpp
PropulsionModel()
PropulsionModel(const PropulsionParams& params)
```

#### Methods
```cpp
void initialize(const PropulsionParams& params)
void reset()
PropulsionOutput compute(real_T throttle, real_T velocity, real_T angular_velocity = 0)
const PropulsionParams& getParams() const
PropulsionType getType() const
real_T getMaxThrust() const
real_T getMaxPower() const
void setParams(const PropulsionParams& params)
void setFluidDensity(real_T density)
```

#### PropulsionParams Structure
```cpp
struct PropulsionParams {
    PropulsionType type;
    real_T diameter;           // [m]
    real_T pitch;              // [m]
    real_T blade_area;         // [m²]
    real_T num_blades;
    real_T max_rpm;
    real_T efficiency;         // [0-1]
    real_T gear_ratio;
    real_T duct_ratio;
    real_T air_density;        // [kg/m³]
    real_T disk_area;          // [m²] (computed)
};
```

#### PropulsionOutput Structure
```cpp
struct PropulsionOutput {
    real_T thrust;             // [N]
    real_T torque;             // [N⋅m]
    real_T power;              // [W]
    real_T efficiency;         // [0-1]
    real_T rpm;
    real_T advance_ratio;      // J = V/(n⋅D)
};
```

### FormationControl Class

#### Constructor
```cpp
FormationControl()
FormationControl(const FormationParams& params)
```

#### Methods
```cpp
void initialize(const FormationParams& params)
void reset()
FormationCommand computeCommand(
    int vehicle_id,
    const VehicleState& current_state,
    const std::vector<VehicleState>& all_states,
    const VehicleState& leader_state)
void setCustomFormation(const std::vector<Vector3r>& positions)
Vector3r getDesiredPosition(int vehicle_id, const VehicleState& leader_state, int num_vehicles) const
const FormationParams& getParams() const
void setParams(const FormationParams& params)
void setFormationType(FormationType type)
void setSpacing(real_T spacing)
```

#### FormationParams Structure
```cpp
struct FormationParams {
    FormationType type;
    real_T spacing;              // [m]
    real_T collision_radius;     // [m]
    real_T max_velocity;         // [m/s]
    real_T max_acceleration;     // [m/s²]
    real_T k_position;
    real_T k_velocity;
    real_T k_separation;
    real_T k_cohesion;
    real_T k_alignment;
    real_T formation_radius;     // [m] for circle
    real_T formation_angle;      // [rad] for wedge
};
```

#### VehicleState Structure
```cpp
struct VehicleState {
    Vector3r position;
    Vector3r velocity;
    Quaternionr orientation;
    int id;
};
```

#### FormationCommand Structure
```cpp
struct FormationCommand {
    Vector3r desired_velocity;
    Vector3r desired_acceleration;
    Quaternionr desired_orientation;
};
```

---

## Performance Considerations

### PropulsionModel
- **Computational Cost**: O(1) per vehicle
- **Update Frequency**: Can run at high rate (100-1000 Hz)
- **Memory**: Minimal (~200 bytes per instance)

### FormationControl
- **Computational Cost**: O(n) where n = number of vehicles
- **Update Frequency**: Typically 10-50 Hz sufficient
- **Memory**: O(n) for vehicle states

### Optimization Tips
1. **Spatial Partitioning**: For large swarms (>50 vehicles), use spatial hashing for neighbor queries
2. **Update Rate**: Formation control can run slower than physics (10-20 Hz vs 100+ Hz)
3. **Lazy Evaluation**: Cache formation positions if leader state hasn't changed

---

## References

### Propulsion
- Wageningen B-Series Propeller Theory
- Momentum Theory for Rotors (Leishman, 2006)
- Marine Propeller Hydrodynamics (Carlton, 2012)

### Formation Control
- Flocking Behaviors (Reynolds, 1987)
- Potential Fields for Multi-Robot Systems (Khatib, 1986)
- Leader-Follower Formation Control (Desai et al., 1998)

---

## License

Copyright (c) nervosys. All rights reserved.
Licensed under the MIT License.
