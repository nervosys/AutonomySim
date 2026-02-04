//! MuJoCo Backend Implementation
//!
//! Provides physics simulation using MuJoCo (Multi-Joint dynamics with Contact).
//! Excellent for contact-rich simulations and robotics research.

use async_trait::async_trait;
use nalgebra::Point3;
use std::collections::HashMap;
use std::sync::{Arc, RwLock};

use autonomysim_core::{
    backend::{
        BackendConfig, BackendType, Material, Position, Ray, RayHit, SceneHandle, SceneObject,
        SimResult, SimulationBackend, Transform,
    },
    sensor::SensorData,
    vehicle::{VehicleControl, VehicleId, VehicleSpec, VehicleState},
    SimError,
};

mod ffi;
use ffi::MuJoCoFFI;

/// Configuration for MuJoCo backend
#[derive(Debug, Clone)]
pub struct MuJoCoConfig {
    /// Timestep in seconds
    pub timestep: f64,
    /// Number of simulation substeps per step
    pub substeps: usize,
    /// Enable contact detection
    pub enable_contact: bool,
    /// Enable joint limits
    pub enable_limits: bool,
    /// Solver iterations
    pub solver_iterations: usize,
    /// Path to MuJoCo model XML (optional)
    pub model_path: Option<String>,
}

impl Default for MuJoCoConfig {
    fn default() -> Self {
        Self {
            timestep: 0.002, // 2ms (500Hz)
            substeps: 1,
            enable_contact: true,
            enable_limits: true,
            solver_iterations: 100,
            model_path: None,
        }
    }
}

/// MuJoCo simulation backend
pub struct MuJoCoBackend {
    /// FFI interface to MuJoCo C API
    ffi: Option<Arc<RwLock<MuJoCoFFI>>>,
    /// Loaded scenes (model files)
    scenes: HashMap<String, MuJoCoSceneHandle>,
    /// Spawned vehicles (bodies in MuJoCo)
    vehicles: HashMap<String, MuJoCoVehicleHandle>,
    /// Current simulation time
    time: f64,
    /// Initialization state
    initialized: bool,
    /// Backend configuration
    config: MuJoCoConfig,
}

#[derive(Debug, Clone)]
struct MuJoCoSceneHandle {
    model_id: i32,
    model_path: String,
    num_bodies: usize,
}

#[derive(Debug, Clone)]
struct MuJoCoVehicleHandle {
    vehicle_id: String,
    body_id: i32,
    actuator_ids: Vec<i32>,
}

impl MuJoCoBackend {
    /// Create a new MuJoCo backend
    pub fn new() -> Self {
        Self {
            ffi: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config: MuJoCoConfig::default(),
        }
    }

    /// Create backend with custom configuration
    pub fn with_config(config: MuJoCoConfig) -> Self {
        Self {
            ffi: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config,
        }
    }

    /// Get current configuration
    pub fn config(&self) -> &MuJoCoConfig {
        &self.config
    }
}

impl Default for MuJoCoBackend {
    fn default() -> Self {
        Self::new()
    }
}

#[async_trait]
impl SimulationBackend for MuJoCoBackend {
    fn name(&self) -> &str {
        "MuJoCo"
    }

    fn is_initialized(&self) -> bool {
        self.initialized
    }

    fn get_time(&self) -> f64 {
        self.time
    }

    async fn initialize(&mut self, config: BackendConfig) -> SimResult<()> {
        if self.initialized {
            return Err(SimError::BackendError("Already initialized".to_string()));
        }

        // Parse custom config if provided
        if !config.custom_config.is_null() {
            if let Some(timestep) = config.custom_config.get("timestep") {
                if let Some(dt) = timestep.as_f64() {
                    self.config.timestep = dt;
                }
            }
            if let Some(substeps) = config.custom_config.get("substeps") {
                if let Some(n) = substeps.as_u64() {
                    self.config.substeps = n as usize;
                }
            }
            if let Some(iters) = config.custom_config.get("solver_iterations") {
                if let Some(n) = iters.as_u64() {
                    self.config.solver_iterations = n as usize;
                }
            }
        }

        // Initialize MuJoCo FFI
        let ffi = MuJoCoFFI::new(self.config.clone())?;
        self.ffi = Some(Arc::new(RwLock::new(ffi)));

        self.initialized = true;
        Ok(())
    }

    async fn shutdown(&mut self) -> SimResult<()> {
        if !self.initialized {
            return Ok(());
        }

        // Cleanup all vehicles
        let vehicle_ids: Vec<_> = self.vehicles.keys().cloned().collect();
        for vehicle_id in vehicle_ids {
            let _ = self.remove_vehicle(&vehicle_id).await;
        }

        // Cleanup FFI
        if let Some(ffi) = self.ffi.take() {
            ffi.write().unwrap().shutdown()?;
        }

        self.scenes.clear();
        self.initialized = false;
        Ok(())
    }

    async fn step(&mut self, delta_time: f64) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .write()
            .unwrap();

        // Step the simulation with substeps
        let num_steps = (delta_time / self.config.timestep).ceil() as usize;
        for _ in 0..num_steps {
            ffi.step()?;
        }

        self.time += delta_time;
        Ok(())
    }

    async fn load_scene(&mut self, scene_path: &str) -> SimResult<SceneHandle> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let mut ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .write()
            .unwrap();

        // Load MuJoCo XML model
        let model_id = ffi.load_model(scene_path)?;
        let num_bodies = ffi.get_num_bodies()?;

        let handle = MuJoCoSceneHandle {
            model_id,
            model_path: scene_path.to_string(),
            num_bodies,
        };

        let scene_id = format!("mujoco_scene_{}", self.scenes.len());
        self.scenes.insert(scene_id.clone(), handle);

        Ok(SceneHandle::new(scene_id, BackendType::MuJoCo))
    }

    fn get_scene_bounds(&self, _scene: &SceneHandle) -> SimResult<(Position, Position)> {
        // Return default bounds for now
        Ok((
            Point3::new(-100.0, -100.0, 0.0),
            Point3::new(100.0, 100.0, 50.0),
        ))
    }

    fn add_object(&mut self, _scene: &SceneHandle, _object: SceneObject) -> SimResult<String> {
        // MuJoCo models are typically defined in XML
        // Dynamic object addition requires recompiling the model
        Ok("object_id".to_string())
    }

    fn remove_object(&mut self, _scene: &SceneHandle, _object_id: &str) -> SimResult<()> {
        // MuJoCo doesn't support dynamic object removal
        Ok(())
    }

    fn update_transform(
        &mut self,
        _scene: &SceneHandle,
        _object_id: &str,
        _transform: Transform,
    ) -> SimResult<()> {
        // Can set body positions/orientations in MuJoCo
        Ok(())
    }

    fn cast_ray(&self, _scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .read()
            .unwrap();

        // Cast ray using MuJoCo's collision detection
        ffi.cast_ray(&ray.origin, &ray.direction, ray.max_distance)
    }

    fn cast_rays(&self, scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>> {
        // MuJoCo can batch process rays efficiently
        rays.iter().map(|ray| self.cast_ray(scene, ray)).collect()
    }

    fn get_objects(&self, _scene: &SceneHandle) -> SimResult<Vec<SceneObject>> {
        // Return list of bodies in MuJoCo model
        Ok(Vec::new())
    }

    async fn spawn_vehicle(&mut self, spec: VehicleSpec) -> SimResult<VehicleId> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let mut ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .write()
            .unwrap();

        // In MuJoCo, vehicles are typically defined in the XML model
        // Here we'll create a body and associated actuators
        let body_id = ffi.create_body(&spec)?;
        let actuator_ids = ffi.create_actuators(&spec, body_id)?;

        let handle = MuJoCoVehicleHandle {
            vehicle_id: spec.vehicle_id.clone(),
            body_id,
            actuator_ids,
        };

        self.vehicles.insert(spec.vehicle_id.clone(), handle);

        Ok(spec.vehicle_id)
    }

    async fn remove_vehicle(&mut self, vehicle_id: &str) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?
            .clone();

        let mut ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .write()
            .unwrap();

        // Remove body and actuators from simulation
        ffi.remove_body(handle.body_id)?;

        self.vehicles.remove(vehicle_id);
        Ok(())
    }

    fn get_vehicle_state(&self, vehicle_id: &str) -> SimResult<VehicleState> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;

        let ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .read()
            .unwrap();

        // Get state from MuJoCo body
        ffi.get_body_state(handle.body_id, vehicle_id)
    }

    fn set_vehicle_control(&mut self, vehicle_id: &str, control: VehicleControl) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?
            .clone();

        let mut ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .write()
            .unwrap();

        // Set actuator controls in MuJoCo
        ffi.set_actuator_controls(&handle.actuator_ids, &control)?;

        Ok(())
    }

    fn get_sensor_data(&self, vehicle_id: &str, sensor_id: &str) -> SimResult<SensorData> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "MuJoCo backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;

        let ffi = self
            .ffi
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("FFI not available".to_string()))?
            .read()
            .unwrap();

        // Get sensor data from MuJoCo sensor
        ffi.get_sensor_data(handle.body_id, sensor_id)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_backend_creation() {
        let backend = MuJoCoBackend::new();
        assert_eq!(backend.name(), "MuJoCo");
        assert!(!backend.is_initialized());
    }

    #[test]
    fn test_config_defaults() {
        let config = MuJoCoConfig::default();
        assert_eq!(config.timestep, 0.002);
        assert_eq!(config.substeps, 1);
        assert_eq!(config.solver_iterations, 100);
        assert!(config.enable_contact);
    }

    #[test]
    fn test_custom_config() {
        let config = MuJoCoConfig {
            timestep: 0.001,
            substeps: 5,
            enable_contact: false,
            enable_limits: false,
            solver_iterations: 200,
            model_path: Some("/path/to/model.xml".to_string()),
        };

        let backend = MuJoCoBackend::with_config(config.clone());
        assert_eq!(backend.config().timestep, 0.001);
        assert_eq!(backend.config().substeps, 5);
        assert!(!backend.config().enable_contact);
    }
}
