//! Vehicle types and control interface

use crate::backend::{Position, Rotation, Transform, Vec3};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// Unique identifier for a vehicle
pub type VehicleId = String;

/// Vehicle type enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum VehicleType {
    /// Multirotor drone (quadcopter, hexacopter, etc.)
    Multirotor,
    /// Fixed-wing aircraft
    FixedWing,
    /// Ground vehicle (car, rover, etc.)
    Car,
    /// Hybrid VTOL
    VTOL,
    /// Custom vehicle type
    Custom,
}

/// Vehicle state information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VehicleState {
    pub vehicle_id: VehicleId,
    pub timestamp: f64,
    pub transform: Transform,
    pub linear_velocity: Vec3,
    pub angular_velocity: Vec3,
    pub linear_acceleration: Vec3,
    pub angular_acceleration: Vec3,
    pub battery_level: f64, // 0.0 to 1.0
    pub is_grounded: bool,
    pub collision_info: Option<CollisionInfo>,
}

/// Collision information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CollisionInfo {
    pub has_collided: bool,
    pub collision_count: u32,
    pub impact_point: Position,
    pub impact_normal: Vec3,
    pub impact_force: Vec3,
    pub penetration_depth: f64,
}

/// Vehicle control inputs
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VehicleControl {
    /// Throttle (0.0 to 1.0)
    pub throttle: f64,
    /// Steering (-1.0 to 1.0)
    pub steering: f64,
    /// Brake (0.0 to 1.0)
    pub brake: f64,
    /// Pitch (-1.0 to 1.0)
    pub pitch: f64,
    /// Roll (-1.0 to 1.0)
    pub roll: f64,
    /// Yaw (-1.0 to 1.0)
    pub yaw: f64,
    /// Manual gear (optional)
    pub manual_gear: Option<i32>,
    /// Is manual gear enabled
    pub is_manual_gear: bool,
}

impl Default for VehicleControl {
    fn default() -> Self {
        Self {
            throttle: 0.0,
            steering: 0.0,
            brake: 0.0,
            pitch: 0.0,
            roll: 0.0,
            yaw: 0.0,
            manual_gear: None,
            is_manual_gear: false,
        }
    }
}

impl VehicleControl {
    /// Create control for hovering (multirotor)
    pub fn hover() -> Self {
        Self {
            throttle: 0.5,
            ..Default::default()
        }
    }
    
    /// Create control for forward flight
    pub fn forward(speed: f64) -> Self {
        Self {
            throttle: 0.5,
            pitch: -speed.clamp(-1.0, 1.0),
            ..Default::default()
        }
    }
}

/// Vehicle specification for spawning
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VehicleSpec {
    pub vehicle_id: VehicleId,
    pub vehicle_type: VehicleType,
    pub initial_transform: Transform,
    pub parameters: VehicleParameters,
    pub sensors: Vec<SensorSpec>,
}

/// Vehicle physical parameters
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VehicleParameters {
    pub mass: f64,
    pub inertia: Vec3,
    pub drag_coefficient: f64,
    pub max_thrust: f64,
    pub max_torque: Vec3,
    pub sensor_offsets: HashMap<String, Transform>,
}

impl Default for VehicleParameters {
    fn default() -> Self {
        Self {
            mass: 1.0,
            inertia: Vec3::new(1.0, 1.0, 1.0),
            drag_coefficient: 0.3,
            max_thrust: 100.0,
            max_torque: Vec3::new(10.0, 10.0, 10.0),
            sensor_offsets: HashMap::new(),
        }
    }
}

/// Sensor specification
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SensorSpec {
    pub sensor_id: String,
    pub sensor_type: SensorType,
    pub update_rate_hz: f64,
    pub enabled: bool,
}

/// Sensor type enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum SensorType {
    /// RGB camera
    Camera,
    /// Depth camera
    DepthCamera,
    /// Segmentation camera
    SegmentationCamera,
    /// LiDAR
    Lidar,
    /// IMU (Inertial Measurement Unit)
    Imu,
    /// GPS
    Gps,
    /// Magnetometer
    Magnetometer,
    /// Barometer
    Barometer,
    /// Distance sensor (ultrasonic, IR)
    DistanceSensor,
    /// RF antenna sensor
    RfAntenna,
}

#[cfg(test)]
mod tests {
    use super::*;
    use nalgebra::{Point3, UnitQuaternion};

    #[test]
    fn test_vehicle_control_default() {
        let control = VehicleControl::default();
        assert_eq!(control.throttle, 0.0);
        assert_eq!(control.steering, 0.0);
    }

    #[test]
    fn test_vehicle_control_hover() {
        let control = VehicleControl::hover();
        assert_eq!(control.throttle, 0.5);
    }

    #[test]
    fn test_vehicle_spec() {
        let spec = VehicleSpec {
            vehicle_id: "drone1".to_string(),
            vehicle_type: VehicleType::Multirotor,
            initial_transform: Transform::new(
                Point3::new(0.0, 0.0, 10.0),
                UnitQuaternion::identity(),
            ),
            parameters: VehicleParameters::default(),
            sensors: vec![
                SensorSpec {
                    sensor_id: "camera1".to_string(),
                    sensor_type: SensorType::Camera,
                    update_rate_hz: 30.0,
                    enabled: true,
                }
            ],
        };
        
        assert_eq!(spec.vehicle_type, VehicleType::Multirotor);
        assert_eq!(spec.sensors.len(), 1);
    }
}
