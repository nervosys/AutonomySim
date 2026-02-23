// Core trait definitions for simulation backends

use async_trait::async_trait;
use nalgebra::{Point3, Vector3};
use serde::{Deserialize, Serialize};
use thiserror::Error;

/// Result type for simulation operations
pub type SimResult<T> = Result<T, SimError>;

/// Errors that can occur during simulation
#[derive(Error, Debug)]
pub enum SimError {
    #[error("Backend not initialized: {0}")]
    NotInitialized(String),

    #[error("Backend error: {0}")]
    BackendError(String),

    #[error("Invalid configuration: {0}")]
    InvalidConfig(String),

    #[error("Scene not found: {0}")]
    SceneNotFound(String),

    #[error("IO error: {0}")]
    IoError(#[from] std::io::Error),

    #[error("Serialization error: {0}")]
    SerializationError(#[from] serde_json::Error),
}

/// 3D position in simulation world
pub type Position = Point3<f64>;

/// 3D vector
pub type Vec3 = Vector3<f64>;

/// Rotation quaternion (unit quaternion for 3D rotations)
pub type Rotation = nalgebra::UnitQuaternion<f64>;

/// Transform (position + rotation)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Transform {
    pub position: Position,
    pub rotation: Rotation,
}

impl Transform {
    pub fn new(position: Position, rotation: Rotation) -> Self {
        Self { position, rotation }
    }

    pub fn identity() -> Self {
        Self {
            position: Position::origin(),
            rotation: Rotation::identity(),
        }
    }
}

/// Material properties for RF propagation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Material {
    pub name: String,
    pub permittivity: f64,           // Relative permittivity (εr)
    pub conductivity: f64,           // Conductivity (σ) in S/m
    pub permeability: f64,           // Relative permeability (μr)
    pub roughness: f64,              // Surface roughness in meters
    pub reflection_coefficient: f64, // Fresnel reflection coefficient
}

impl Material {
    pub fn air() -> Self {
        Self {
            name: "Air".to_string(),
            permittivity: 1.0,
            conductivity: 0.0,
            permeability: 1.0,
            roughness: 0.0,
            reflection_coefficient: 0.0,
        }
    }

    pub fn concrete() -> Self {
        Self {
            name: "Concrete".to_string(),
            permittivity: 5.0,
            conductivity: 0.01,
            permeability: 1.0,
            roughness: 0.001,
            reflection_coefficient: 0.3,
        }
    }

    pub fn metal() -> Self {
        Self {
            name: "Metal".to_string(),
            permittivity: 1.0,
            conductivity: 1e7,
            permeability: 1.0,
            roughness: 0.0001,
            reflection_coefficient: 0.95,
        }
    }
}

/// Scene object in the simulation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SceneObject {
    pub id: String,
    pub name: String,
    pub transform: Transform,
    pub geometry: Geometry,
    pub material: Material,
}

/// Geometry types
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Geometry {
    Box {
        size: Vec3,
    },
    Sphere {
        radius: f64,
    },
    Cylinder {
        radius: f64,
        height: f64,
    },
    Mesh {
        vertices: Vec<Position>,
        indices: Vec<[u32; 3]>,
    },
}

/// Ray for ray tracing
#[derive(Debug, Clone)]
pub struct Ray {
    pub origin: Position,
    pub direction: Vec3,
    pub max_distance: f64,
}

impl Ray {
    pub fn new(origin: Position, direction: Vec3) -> Self {
        Self {
            origin,
            direction: direction.normalize(),
            max_distance: f64::INFINITY,
        }
    }

    pub fn at(&self, t: f64) -> Position {
        self.origin + self.direction * t
    }
}

/// Ray intersection result
#[derive(Debug, Clone)]
pub struct RayHit {
    pub distance: f64,
    pub position: Position,
    pub normal: Vec3,
    pub material: Material,
    pub object_id: String,
}

/// Main simulation backend trait
#[async_trait]
pub trait SimulationBackend: Send + Sync {
    /// Get the name of this backend
    fn name(&self) -> &str;

    /// Initialize the backend
    async fn initialize(&mut self, config: BackendConfig) -> SimResult<()>;

    /// Shutdown the backend
    async fn shutdown(&mut self) -> SimResult<()>;

    /// Check if backend is initialized
    fn is_initialized(&self) -> bool;

    /// Load a scene
    async fn load_scene(&mut self, scene_path: &str) -> SimResult<SceneHandle>;

    /// Get scene bounds
    fn get_scene_bounds(&self, scene: &SceneHandle) -> SimResult<(Position, Position)>;

    /// Add an object to the scene
    fn add_object(&mut self, scene: &SceneHandle, object: SceneObject) -> SimResult<String>;

    /// Remove an object from the scene
    fn remove_object(&mut self, scene: &SceneHandle, object_id: &str) -> SimResult<()>;

    /// Update object transform
    fn update_transform(
        &mut self,
        scene: &SceneHandle,
        object_id: &str,
        transform: Transform,
    ) -> SimResult<()>;

    /// Cast a ray through the scene
    fn cast_ray(&self, scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>>;

    /// Cast multiple rays (batch operation)
    fn cast_rays(&self, scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>>;

    /// Get all objects in the scene
    fn get_objects(&self, scene: &SceneHandle) -> SimResult<Vec<SceneObject>>;

    /// Step the simulation (if time-based)
    async fn step(&mut self, delta_time: f64) -> SimResult<()>;

    /// Get current simulation time
    fn get_time(&self) -> f64;

    /// Spawn a vehicle in the scene
    async fn spawn_vehicle(
        &mut self,
        spec: crate::vehicle::VehicleSpec,
    ) -> SimResult<crate::vehicle::VehicleId>;

    /// Remove a vehicle from the scene
    async fn remove_vehicle(&mut self, vehicle_id: &str) -> SimResult<()>;

    /// Get vehicle state
    fn get_vehicle_state(&self, vehicle_id: &str) -> SimResult<crate::vehicle::VehicleState>;

    /// Set vehicle control inputs
    fn set_vehicle_control(
        &mut self,
        vehicle_id: &str,
        control: crate::vehicle::VehicleControl,
    ) -> SimResult<()>;

    /// Get sensor data from vehicle
    fn get_sensor_data(
        &self,
        vehicle_id: &str,
        sensor_id: &str,
    ) -> SimResult<crate::sensor::SensorData>;
}

/// Backend configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BackendConfig {
    pub backend_type: BackendType,
    pub scene_path: Option<String>,
    pub enable_physics: bool,
    pub enable_rendering: bool,
    pub parallel_processing: bool,
    pub num_threads: Option<usize>,
    pub custom_config: serde_json::Value,
}

impl Default for BackendConfig {
    fn default() -> Self {
        Self {
            backend_type: BackendType::Native,
            scene_path: None,
            enable_physics: false,
            enable_rendering: false,
            parallel_processing: true,
            num_threads: None,
            custom_config: serde_json::Value::Null,
        }
    }
}

/// Backend type enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum BackendType {
    /// Native Rust ray tracing (fastest for RF)
    Native,
    /// Unreal Engine 5
    UnrealEngine5,
    /// NVIDIA Isaac Lab
    IsaacLab,
    /// MuJoCo physics engine
    MuJoCo,
    /// NVIDIA Warp
    Warp,
}

impl BackendType {
    pub fn as_str(&self) -> &str {
        match self {
            BackendType::Native => "native",
            BackendType::UnrealEngine5 => "unreal_engine_5",
            BackendType::IsaacLab => "isaac_lab",
            BackendType::MuJoCo => "mujoco",
            BackendType::Warp => "warp",
        }
    }
}

/// Handle to a loaded scene
#[derive(Debug, Clone)]
pub struct SceneHandle {
    pub id: String,
    pub backend_type: BackendType,
}

impl SceneHandle {
    pub fn new(id: String, backend_type: BackendType) -> Self {
        Self { id, backend_type }
    }
}
