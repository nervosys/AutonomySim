//! AutonomySim Core Library
//!
//! This crate provides core types, traits, and utilities for the AutonomySim
//! simulation framework. It defines the backend abstraction layer that allows
//! AutonomySim to work with multiple simulation engines including:
//!
//! - Native Rust ray tracing
//! - Unreal Engine 5
//! - NVIDIA Isaac Lab
//! - MuJoCo
//! - NVIDIA Warp
//!
//! # Architecture
//!
//! The core architecture consists of:
//!
//! - **Backend Trait**: Abstract interface for simulation engines
//! - **Scene Management**: Loading and manipulating simulation scenes
//! - **Ray Tracing**: Efficient ray casting for RF propagation
//! - **Materials**: Physical material properties for RF simulation
//! - **Transforms**: 3D position and rotation handling
//!
//! # Example
//!
//! ```rust,no_run
//! use autonomysim_core::prelude::*;
//!
//! #[tokio::main]
//! async fn main() -> SimResult<()> {
//!     // Create a backend
//!     let mut backend = BackendFactory::create(BackendType::Native)?;
//!     
//!     // Initialize with config
//!     let config = BackendConfig::default();
//!     backend.initialize(config).await?;
//!     
//!     // Load a scene
//!     let scene = backend.load_scene("city.obj").await?;
//!     
//!     // Cast a ray
//!     let ray = Ray::new(
//!         Position::new(0.0, 0.0, 10.0),
//!         Vec3::new(0.0, 0.0, -1.0)
//!     );
//!     
//!     if let Some(hit) = backend.cast_ray(&scene, &ray)? {
//!         println!("Hit at distance: {}", hit.distance);
//!     }
//!     
//!     Ok(())
//! }
//! ```

pub mod backend;
pub mod fpv;
pub mod native;
pub mod sensor;
pub mod vehicle;

// Re-exports for convenience
pub use backend::{
    BackendConfig, BackendFactory, BackendType, Geometry, Material, Position, Ray, RayHit,
    Rotation, SceneHandle, SceneObject, SimError, SimResult, SimulationBackend, Transform, Vec3,
};
pub use sensor::{CameraData, GpsData, ImuData, LidarData, SensorData};
pub use vehicle::{VehicleControl, VehicleId, VehicleSpec, VehicleState, VehicleType};
pub use fpv::{
    FpvCameraConfig, FpvDroneConfig, FpvFlightMode, FpvOsd, FpvPhysics, FpvState,
    FpvStickInput, PidGains, RatesProfile,
};

/// Prelude module for common imports
pub mod prelude {
    pub use crate::backend::{
        BackendConfig, BackendFactory, BackendType, Geometry, Material, Position, Ray, RayHit,
        Rotation, SceneHandle, SceneObject, SimError, SimResult, SimulationBackend, Transform,
        Vec3,
    };
    pub use crate::sensor::{CameraData, GpsData, ImuData, LidarData, SensorData};
    pub use crate::vehicle::{VehicleControl, VehicleId, VehicleSpec, VehicleState, VehicleType};
    pub use crate::fpv::{
        FpvCameraConfig, FpvDroneConfig, FpvFlightMode, FpvOsd, FpvPhysics, FpvState,
        FpvStickInput, PidGains, RatesProfile,
    };
    pub use nalgebra::{Point3, Quaternion, UnitQuaternion, Vector3};
}

#[cfg(test)]
mod tests {
    use super::*;
    use nalgebra::{Point3, UnitQuaternion, Vector3};

    #[test]
    fn test_transform_identity() {
        let transform = Transform::identity();
        assert_eq!(transform.position, Point3::origin());
        assert_eq!(transform.rotation, UnitQuaternion::identity());
    }

    #[test]
    fn test_material_air() {
        let air = Material::air();
        assert_eq!(air.permittivity, 1.0);
        assert_eq!(air.conductivity, 0.0);
    }

    #[test]
    fn test_ray_creation() {
        let origin = Point3::new(0.0, 0.0, 0.0);
        let direction = Vector3::new(1.0, 0.0, 0.0);
        let ray = Ray::new(origin, direction);

        assert_eq!(ray.origin, origin);
        assert_eq!(ray.direction, direction.normalize());
        assert_eq!(ray.max_distance, f64::INFINITY);
    }

    #[test]
    fn test_ray_at() {
        let origin = Point3::new(0.0, 0.0, 0.0);
        let direction = Vector3::new(1.0, 0.0, 0.0);
        let ray = Ray::new(origin, direction);

        let point = ray.at(5.0);
        assert_eq!(point, Point3::new(5.0, 0.0, 0.0));
    }

    #[test]
    fn test_backend_type_as_str() {
        assert_eq!(BackendType::Native.as_str(), "native");
        assert_eq!(BackendType::UnrealEngine5.as_str(), "unreal_engine_5");
        assert_eq!(BackendType::IsaacLab.as_str(), "isaac_lab");
    }
}
