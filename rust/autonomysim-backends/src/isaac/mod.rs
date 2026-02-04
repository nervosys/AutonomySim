//! Isaac Lab Backend Implementation
//!
//! Provides GPU-accelerated physics simulation using NVIDIA Isaac Lab.
//! Supports parallel simulation of multiple environments and vehicles.

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

mod python_bridge;
use python_bridge::IsaacLabPythonBridge;

/// Configuration for Isaac Lab backend
#[derive(Debug, Clone)]
pub struct IsaacLabConfig {
    /// Number of parallel environments to simulate
    pub num_envs: usize,
    /// Device to use (cuda:0, cpu)
    pub device: String,
    /// Physics timestep in seconds
    pub physics_dt: f64,
    /// Enable GPU pipeline
    pub enable_gpu_pipeline: bool,
    /// Enable scene graph
    pub enable_scene_graph: bool,
    /// Python executable path (optional)
    pub python_path: Option<String>,
}

impl Default for IsaacLabConfig {
    fn default() -> Self {
        Self {
            num_envs: 1,
            device: "cuda:0".to_string(),
            physics_dt: 0.01,
            enable_gpu_pipeline: true,
            enable_scene_graph: true,
            python_path: None,
        }
    }
}

/// Isaac Lab simulation backend
pub struct IsaacLabBackend {
    /// Python bridge for Isaac Lab API
    bridge: Option<Arc<RwLock<IsaacLabPythonBridge>>>,
    /// Loaded scenes
    scenes: HashMap<String, IsaacSceneHandle>,
    /// Spawned vehicles
    vehicles: HashMap<String, IsaacVehicleHandle>,
    /// Current simulation time
    time: f64,
    /// Initialization state
    initialized: bool,
    /// Backend configuration
    config: IsaacLabConfig,
}

#[derive(Debug, Clone)]
struct IsaacSceneHandle {
    scene_id: String,
    scene_path: String,
    num_prims: usize,
}

#[derive(Debug, Clone)]
struct IsaacVehicleHandle {
    vehicle_id: String,
    env_id: usize,
    prim_path: String,
}

impl IsaacLabBackend {
    /// Create a new Isaac Lab backend
    pub fn new() -> Self {
        Self {
            bridge: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config: IsaacLabConfig::default(),
        }
    }

    /// Create backend with custom configuration
    pub fn with_config(config: IsaacLabConfig) -> Self {
        Self {
            bridge: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config,
        }
    }

    /// Get current configuration
    pub fn config(&self) -> &IsaacLabConfig {
        &self.config
    }
}

impl Default for IsaacLabBackend {
    fn default() -> Self {
        Self::new()
    }
}

#[async_trait]
impl SimulationBackend for IsaacLabBackend {
    fn name(&self) -> &str {
        "IsaacLab"
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
            if let Some(num_envs) = config.custom_config.get("num_envs") {
                if let Some(num) = num_envs.as_u64() {
                    self.config.num_envs = num as usize;
                }
            }
            if let Some(device) = config.custom_config.get("device") {
                if let Some(dev) = device.as_str() {
                    self.config.device = dev.to_string();
                }
            }
            if let Some(physics_dt) = config.custom_config.get("physics_dt") {
                if let Some(dt) = physics_dt.as_f64() {
                    self.config.physics_dt = dt;
                }
            }
        }

        // Initialize Python bridge
        let bridge = IsaacLabPythonBridge::new(self.config.clone())?;
        self.bridge = Some(Arc::new(RwLock::new(bridge)));

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

        // Cleanup Python bridge
        if let Some(bridge) = self.bridge.take() {
            bridge.write().unwrap().shutdown()?;
        }

        self.scenes.clear();
        self.initialized = false;
        Ok(())
    }

    async fn step(&mut self, delta_time: f64) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .read()
            .unwrap();

        // Step the simulation through Python bridge
        bridge.step(delta_time)?;

        self.time += delta_time;
        Ok(())
    }

    async fn load_scene(&mut self, scene_path: &str) -> SimResult<SceneHandle> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let mut bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .write()
            .unwrap();

        // Load scene through Python bridge
        let scene_id = bridge.load_scene(scene_path)?;
        let num_prims = bridge.get_scene_prim_count(&scene_id)?;

        let handle = IsaacSceneHandle {
            scene_id: scene_id.clone(),
            scene_path: scene_path.to_string(),
            num_prims,
        };

        self.scenes.insert(scene_id.clone(), handle);

        Ok(SceneHandle::new(scene_id, BackendType::IsaacLab))
    }

    fn get_scene_bounds(&self, _scene: &SceneHandle) -> SimResult<(Position, Position)> {
        // Return default bounds for now
        Ok((
            Point3::new(-1000.0, -1000.0, 0.0),
            Point3::new(1000.0, 1000.0, 100.0),
        ))
    }

    fn add_object(&mut self, _scene: &SceneHandle, _object: SceneObject) -> SimResult<String> {
        // Placeholder
        Ok("object_id".to_string())
    }

    fn remove_object(&mut self, _scene: &SceneHandle, _object_id: &str) -> SimResult<()> {
        // Placeholder
        Ok(())
    }

    fn update_transform(
        &mut self,
        _scene: &SceneHandle,
        _object_id: &str,
        _transform: Transform,
    ) -> SimResult<()> {
        // Placeholder
        Ok(())
    }

    fn cast_ray(&self, _scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .read()
            .unwrap();

        // Cast ray through Python bridge (uses Isaac's GPU raycasting)
        let result = bridge.cast_ray(&ray.origin, &ray.direction, ray.max_distance)?;

        Ok(result.map(|(distance, normal, position)| RayHit {
            distance,
            position,
            normal,
            object_id: "ground".to_string(),
            material: Material::air(), // Isaac doesn't expose material properties easily
        }))
    }

    fn cast_rays(&self, scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>> {
        // Default implementation: cast rays one by one
        rays.iter().map(|ray| self.cast_ray(scene, ray)).collect()
    }

    fn get_objects(&self, _scene: &SceneHandle) -> SimResult<Vec<SceneObject>> {
        // Placeholder
        Ok(Vec::new())
    }

    async fn spawn_vehicle(&mut self, spec: VehicleSpec) -> SimResult<VehicleId> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let mut bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .write()
            .unwrap();

        // Determine environment ID (round-robin distribution)
        let env_id = self.vehicles.len() % self.config.num_envs;

        // Spawn vehicle through Python bridge
        let prim_path = bridge.spawn_vehicle(env_id, &spec)?;

        let handle = IsaacVehicleHandle {
            vehicle_id: spec.vehicle_id.clone(),
            env_id,
            prim_path,
        };

        self.vehicles.insert(spec.vehicle_id.clone(), handle);

        Ok(spec.vehicle_id)
    }

    async fn remove_vehicle(&mut self, vehicle_id: &str) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?
            .clone();

        let mut bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .write()
            .unwrap();

        // Remove vehicle through Python bridge
        bridge.remove_vehicle(&handle.prim_path)?;

        self.vehicles.remove(vehicle_id);
        Ok(())
    }

    fn get_vehicle_state(&self, vehicle_id: &str) -> SimResult<VehicleState> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;

        let bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .read()
            .unwrap();

        // Get state through Python bridge
        bridge.get_vehicle_state(&handle.prim_path)
    }

    fn set_vehicle_control(&mut self, vehicle_id: &str, control: VehicleControl) -> SimResult<()> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?
            .clone();

        let mut bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .write()
            .unwrap();

        // Set control through Python bridge
        bridge.set_vehicle_control(&handle.prim_path, &control)?;

        Ok(())
    }

    fn get_sensor_data(&self, vehicle_id: &str, sensor_id: &str) -> SimResult<SensorData> {
        if !self.initialized {
            return Err(SimError::NotInitialized(
                "Isaac Lab backend not initialized".to_string(),
            ));
        }

        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;

        let bridge = self
            .bridge
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Bridge not available".to_string()))?
            .read()
            .unwrap();

        // Get sensor data through Python bridge
        bridge.get_sensor_data(&handle.prim_path, sensor_id)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_backend_creation() {
        let backend = IsaacLabBackend::new();
        assert_eq!(backend.name(), "IsaacLab");
        assert!(!backend.is_initialized());
    }

    #[test]
    fn test_config_defaults() {
        let config = IsaacLabConfig::default();
        assert_eq!(config.num_envs, 1);
        assert_eq!(config.device, "cuda:0");
        assert_eq!(config.physics_dt, 0.01);
        assert!(config.enable_gpu_pipeline);
    }

    #[test]
    fn test_custom_config() {
        let config = IsaacLabConfig {
            num_envs: 16,
            device: "cpu".to_string(),
            physics_dt: 0.02,
            enable_gpu_pipeline: false,
            enable_scene_graph: false,
            python_path: Some("/usr/bin/python3".to_string()),
        };

        let backend = IsaacLabBackend::with_config(config.clone());
        assert_eq!(backend.config().num_envs, 16);
        assert_eq!(backend.config().device, "cpu");
    }
}
