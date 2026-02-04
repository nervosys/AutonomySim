//! # NVIDIA Warp Backend
//!
//! GPU-accelerated massively parallel simulation backend using NVIDIA Warp.
//!
//! ## Features
//!
//! - **GPU Compute**: CUDA/HIP kernels for physics simulation
//! - **Massively Parallel**: Simulate thousands of vehicles simultaneously
//! - **Batch Operations**: Vectorized updates on GPU
//! - **SDF Collision**: Signed Distance Field-based collision detection
//! - **GPU Ray Tracing**: Millions of rays per second
//!
//! ## Use Cases
//!
//! - Large-scale multi-agent simulations
//! - Parallel RL environment rollouts
//! - Coverage planning with thousands of agents
//! - Radio propagation at scale (GPU-accelerated ray tracing)
//!
//! ## Architecture
//!
//! ```text
//! ┌──────────────────────────────────────┐
//! │       WarpBackend (Rust)             │
//! ├──────────────────────────────────────┤
//! │  ┌────────────────────────────────┐  │
//! │  │   Python FFI Bridge (PyO3)     │  │
//! │  └────────────────────────────────┘  │
//! │             ↕                         │
//! │  ┌────────────────────────────────┐  │
//! │  │   NVIDIA Warp (Python)         │  │
//! │  │   • wp.launch() kernels        │  │
//! │  │   • GPU arrays (wp.array)      │  │
//! │  │   • SDF collision              │  │
//! │  └────────────────────────────────┘  │
//! │             ↕                         │
//! │  ┌────────────────────────────────┐  │
//! │  │   CUDA/HIP Runtime             │  │
//! │  └────────────────────────────────┘  │
//! └──────────────────────────────────────┘
//! ```

mod ffi;

use async_trait::async_trait;
use ffi::WarpFFI;
use std::{
    collections::HashMap,
    sync::{Arc, RwLock},
};

use autonomysim_core::{
    backend::{BackendConfig, Ray, RayHit, SceneHandle, SimulationBackend},
    sensor::SensorData,
    vehicle::{VehicleControl, VehicleId, VehicleSpec, VehicleState, VehicleType},
    SimError, SimResult,
};

/// Configuration for the Warp backend
#[derive(Debug, Clone)]
pub struct WarpConfig {
    /// Simulation timestep in seconds (default: 0.01 = 100Hz)
    pub timestep: f64,

    /// Number of substeps per timestep (default: 1)
    pub substeps: usize,

    /// Maximum number of parallel vehicles (default: 1000)
    pub max_vehicles: usize,

    /// CUDA device ID (default: 0)
    pub device_id: i32,

    /// Enable GPU ray tracing (default: true)
    pub enable_gpu_raycast: bool,

    /// SDF resolution for collision detection (default: 128)
    pub sdf_resolution: usize,

    /// Path to Warp Python environment
    pub warp_python_path: Option<String>,
}

impl Default for WarpConfig {
    fn default() -> Self {
        Self {
            timestep: 0.01, // 100Hz
            substeps: 1,
            max_vehicles: 1000, // Support 1000 parallel vehicles
            device_id: 0,
            enable_gpu_raycast: true,
            sdf_resolution: 128,
            warp_python_path: None,
        }
    }
}

/// Warp simulation backend
pub struct WarpBackend {
    /// FFI interface to Warp Python
    ffi: Option<Arc<RwLock<WarpFFI>>>,

    /// Loaded scenes (SDF representations)
    scenes: HashMap<String, WarpSceneHandle>,

    /// Spawned vehicles (GPU array indices)
    vehicles: HashMap<String, WarpVehicleHandle>,

    /// Current simulation time
    time: f64,

    /// Initialization status
    initialized: bool,

    /// Configuration
    config: WarpConfig,
}

/// Handle to a scene in Warp
#[derive(Debug, Clone)]
struct WarpSceneHandle {
    /// Scene ID in Warp
    scene_id: i32,

    /// SDF grid dimensions
    sdf_grid_dims: [usize; 3],

    /// Number of objects
    num_objects: usize,
}

/// Handle to a vehicle in Warp
#[derive(Debug, Clone)]
struct WarpVehicleHandle {
    /// Vehicle ID
    vehicle_id: String,

    /// Index in GPU arrays
    array_index: usize,

    /// Vehicle type
    vehicle_type: VehicleType,
}

impl WarpBackend {
    /// Create a new Warp backend with default configuration
    pub fn new() -> Self {
        Self::with_config(WarpConfig::default())
    }

    /// Create a new Warp backend with custom configuration
    pub fn with_config(config: WarpConfig) -> Self {
        Self {
            ffi: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config,
        }
    }
}

impl Default for WarpBackend {
    fn default() -> Self {
        Self::new()
    }
}

#[async_trait]
impl SimulationBackend for WarpBackend {
    fn name(&self) -> &str {
        "NVIDIA Warp (GPU Compute)"
    }

    fn is_initialized(&self) -> bool {
        self.initialized
    }

    fn get_time(&self) -> f64 {
        self.time
    }

    async fn initialize(&mut self, _config: BackendConfig) -> SimResult<()> {
        if self.initialized {
            return Err(SimError::BackendError(
                "Warp backend already initialized".to_string(),
            ));
        }

        // Initialize Warp FFI
        let ffi = WarpFFI::new(self.config.clone())?;
        self.ffi = Some(Arc::new(RwLock::new(ffi)));

        self.initialized = true;
        Ok(())
    }

    async fn shutdown(&mut self) -> SimResult<()> {
        if !self.initialized {
            return Ok(());
        }

        // Shutdown Warp
        if let Some(ffi) = &self.ffi {
            ffi.write().unwrap().shutdown()?;
        }

        self.ffi = None;
        self.scenes.clear();
        self.vehicles.clear();
        self.time = 0.0;
        self.initialized = false;

        Ok(())
    }

    async fn step(&mut self, delta_time: f64) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let ffi = self.ffi.as_ref().unwrap();

        // Calculate number of substeps
        let num_steps = (delta_time / self.config.timestep).ceil() as usize;

        // Step physics on GPU (batch updates all vehicles)
        for _ in 0..num_steps {
            ffi.write().unwrap().step()?;
        }

        self.time += delta_time;

        Ok(())
    }

    async fn load_scene(&mut self, scene_path: &str) -> SimResult<SceneHandle> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let ffi = self.ffi.as_ref().unwrap();

        // Load scene and generate SDF
        let scene_id = ffi.write().unwrap().load_scene(scene_path)?;
        let sdf_grid_dims = ffi.read().unwrap().get_sdf_dimensions()?;
        let num_objects = ffi.read().unwrap().get_num_objects()?;

        let handle = WarpSceneHandle {
            scene_id,
            sdf_grid_dims,
            num_objects,
        };

        let scene_handle = SceneHandle {
            id: scene_path.to_string(),
            backend_type: autonomysim_core::backend::BackendType::Warp,
        };

        self.scenes.insert(scene_path.to_string(), handle);

        Ok(scene_handle)
    }

    fn get_scene_bounds(
        &self,
        _scene: &SceneHandle,
    ) -> SimResult<(nalgebra::Point3<f64>, nalgebra::Point3<f64>)> {
        // Return SDF-based bounds
        Ok((
            nalgebra::Point3::new(-100.0, -100.0, 0.0),
            nalgebra::Point3::new(100.0, 100.0, 50.0),
        ))
    }

    fn add_object(
        &mut self,
        _scene: &SceneHandle,
        _object: autonomysim_core::backend::SceneObject,
    ) -> SimResult<String> {
        Err(SimError::BackendError(
            "Warp backend does not support dynamic object addition (use SDF regeneration)"
                .to_string(),
        ))
    }

    fn remove_object(&mut self, _scene: &SceneHandle, _object_id: &str) -> SimResult<()> {
        Err(SimError::BackendError(
            "Warp backend does not support dynamic object removal (use SDF regeneration)"
                .to_string(),
        ))
    }

    fn update_transform(
        &mut self,
        _scene: &SceneHandle,
        _object_id: &str,
        _transform: autonomysim_core::backend::Transform,
    ) -> SimResult<()> {
        Err(SimError::BackendError(
            "Warp backend does not support dynamic transforms (use SDF regeneration)".to_string(),
        ))
    }

    fn get_objects(
        &self,
        _scene: &SceneHandle,
    ) -> SimResult<Vec<autonomysim_core::backend::SceneObject>> {
        // Warp uses SDF representation, not explicit objects
        Ok(vec![])
    }

    fn cast_ray(&self, _scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let ffi = self.ffi.as_ref().unwrap();

        // Cast ray using GPU SDF
        ffi.read()
            .unwrap()
            .cast_ray(&ray.origin, &ray.direction, ray.max_distance)
    }

    fn cast_rays(&self, _scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let ffi = self.ffi.as_ref().unwrap();

        // Batch ray casting on GPU (highly parallel)
        ffi.read().unwrap().cast_rays(rays)
    }

    async fn spawn_vehicle(&mut self, spec: VehicleSpec) -> SimResult<VehicleId> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let ffi = self.ffi.as_ref().unwrap();

        // Allocate slot in GPU arrays
        let array_index = ffi.write().unwrap().allocate_vehicle(&spec)?;

        let handle = WarpVehicleHandle {
            vehicle_id: spec.vehicle_id.clone(),
            array_index,
            vehicle_type: spec.vehicle_type,
        };

        self.vehicles.insert(spec.vehicle_id.clone(), handle);

        Ok(spec.vehicle_id)
    }

    async fn remove_vehicle(&mut self, vehicle_id: &str) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle '{}' not found", vehicle_id)))?
            .clone();

        let ffi = self.ffi.as_ref().unwrap();

        // Deallocate from GPU arrays
        ffi.write()
            .unwrap()
            .deallocate_vehicle(handle.array_index)?;

        self.vehicles.remove(vehicle_id);

        Ok(())
    }

    fn get_vehicle_state(&self, vehicle_id: &str) -> SimResult<VehicleState> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle '{}' not found", vehicle_id)))?;

        let ffi = self.ffi.as_ref().unwrap();

        // Read from GPU arrays
        ffi.read()
            .unwrap()
            .get_vehicle_state(handle.array_index, vehicle_id)
    }

    fn set_vehicle_control(&mut self, vehicle_id: &str, control: VehicleControl) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle '{}' not found", vehicle_id)))?;

        let ffi = self.ffi.as_ref().unwrap();

        // Update control inputs in GPU arrays
        ffi.write()
            .unwrap()
            .set_vehicle_control(handle.array_index, &control)
    }

    fn get_sensor_data(&self, vehicle_id: &str, sensor_id: &str) -> SimResult<SensorData> {
        if !self.initialized {
            return Err(SimError::BackendError(
                "Warp backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle '{}' not found", vehicle_id)))?;

        let ffi = self.ffi.as_ref().unwrap();

        // Read sensor data from GPU
        ffi.read()
            .unwrap()
            .get_sensor_data(handle.array_index, sensor_id)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_backend_creation() {
        let backend = WarpBackend::new();
        assert_eq!(backend.name(), "NVIDIA Warp (GPU Compute)");
        assert!(!backend.is_initialized());
        assert_eq!(backend.get_time(), 0.0);
    }

    #[test]
    fn test_config_defaults() {
        let config = WarpConfig::default();
        assert_eq!(config.timestep, 0.01);
        assert_eq!(config.substeps, 1);
        assert_eq!(config.max_vehicles, 1000);
        assert_eq!(config.device_id, 0);
        assert!(config.enable_gpu_raycast);
        assert_eq!(config.sdf_resolution, 128);
    }

    #[test]
    fn test_custom_config() {
        let config = WarpConfig {
            timestep: 0.005,
            substeps: 2,
            max_vehicles: 5000,
            device_id: 1,
            enable_gpu_raycast: true,
            sdf_resolution: 256,
            warp_python_path: Some("/path/to/warp".to_string()),
        };

        let backend = WarpBackend::with_config(config.clone());
        assert_eq!(backend.config.timestep, 0.005);
        assert_eq!(backend.config.max_vehicles, 5000);
        assert_eq!(backend.config.device_id, 1);
    }
}
