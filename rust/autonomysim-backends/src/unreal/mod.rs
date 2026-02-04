//! Unreal Engine 5 backend implementation
//!
//! This module provides integration with Unreal Engine 5, communicating
//! via RPC/message passing protocol on port 41451.
//!
//! ## Features
//! - High-performance batched robot spawning and position updates
//! - Robot type differentiation with color coding
//! - Debug visualization (communication links, RF range, paths)
//! - Telemetry streaming (battery, health, signal strength)
//!
//! ## Architecture
//! ```text
//! ┌─────────────────┐    JSON-RPC/TCP    ┌─────────────────┐
//! │  Rust Backend   │◄──────────────────►│  UE5 Plugin     │
//! │  (Simulation)   │    Port 41451      │  (Rendering)    │
//! └─────────────────┘                    └─────────────────┘
//! ```

use async_trait::async_trait;
use autonomysim_core::prelude::*;
use nalgebra::{Point3, UnitQuaternion, Vector3};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::{mpsc, RwLock};
use tracing::{debug, info, warn};

mod connection;
mod protocol;

pub use connection::UnrealConnection;
pub use protocol::{
    DebugLine, DebugSphere, RobotPositionUpdate, RobotSpawnData, RobotTelemetry, RobotType,
    UnrealMessage, UnrealResponse,
};

/// Unreal Engine 5 backend implementation
pub struct UnrealEngine5Backend {
    connection: Option<Arc<RwLock<UnrealConnection>>>,
    scenes: HashMap<String, UnrealSceneHandle>,
    vehicles: HashMap<String, UnrealVehicleHandle>,
    time: f64,
    initialized: bool,
    config: UnrealBackendConfig,
}

/// Configuration for Unreal Engine 5 backend
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnrealBackendConfig {
    pub host: String,
    pub port: u16,
    pub api_version: String,
    pub timeout_seconds: f64,
    pub use_blueprint_api: bool,
}

impl Default for UnrealBackendConfig {
    fn default() -> Self {
        Self {
            host: "127.0.0.1".to_string(),
            port: 41451,
            api_version: "1.0.0".to_string(),
            timeout_seconds: 30.0,
            use_blueprint_api: true,
        }
    }
}

/// Unreal scene handle
struct UnrealSceneHandle {
    scene_id: String,
    level_name: String,
}

/// Unreal vehicle handle
struct UnrealVehicleHandle {
    vehicle_id: String,
    pawn_name: String,
    vehicle_type: VehicleType,
}

impl UnrealEngine5Backend {
    pub fn new() -> Self {
        Self {
            connection: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config: UnrealBackendConfig::default(),
        }
    }

    pub fn with_config(config: UnrealBackendConfig) -> Self {
        Self {
            connection: None,
            scenes: HashMap::new(),
            vehicles: HashMap::new(),
            time: 0.0,
            initialized: false,
            config,
        }
    }

    /// Batch spawn multiple robots in a single RPC call
    pub async fn spawn_robots_batch(
        &self,
        robots: Vec<(i32, f64, f64, f64, f64)>,
    ) -> Result<(), String> {
        let conn = self
            .connection
            .as_ref()
            .ok_or_else(|| "Backend not initialized".to_string())?;

        conn.read()
            .await
            .spawn_robots_batch(robots)
            .await
            .map_err(|e| format!("Failed to spawn robots: {}", e))?;

        Ok(())
    }
}

#[async_trait]
impl SimulationBackend for UnrealEngine5Backend {
    fn name(&self) -> &str {
        "Unreal Engine 5 Backend"
    }

    async fn initialize(&mut self, config: BackendConfig) -> SimResult<()> {
        // Parse custom config if provided
        if !config.custom_config.is_null() {
            if let Ok(unreal_config) =
                serde_json::from_value::<UnrealBackendConfig>(config.custom_config)
            {
                self.config = unreal_config;
            }
        }

        // Establish connection to Unreal Engine
        let connection = UnrealConnection::connect(&self.config.host, self.config.port)
            .await
            .map_err(|e| SimError::BackendError(format!("Failed to connect to UE5: {}", e)))?;

        self.connection = Some(Arc::new(RwLock::new(connection)));
        self.initialized = true;

        Ok(())
    }

    async fn shutdown(&mut self) -> SimResult<()> {
        if let Some(conn) = &self.connection {
            conn.write()
                .await
                .disconnect()
                .await
                .map_err(|e| SimError::BackendError(format!("Disconnect failed: {}", e)))?;
        }

        self.connection = None;
        self.initialized = false;
        Ok(())
    }

    fn is_initialized(&self) -> bool {
        self.initialized
    }

    async fn load_scene(&mut self, scene_path: &str) -> SimResult<SceneHandle> {
        let conn = self
            .connection
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Backend not initialized".to_string()))?;

        // Send load level command to UE5
        let msg = UnrealMessage::LoadLevel {
            level_name: scene_path.to_string(),
        };

        let response = conn
            .read()
            .await
            .send_message(msg)
            .await
            .map_err(|e| SimError::BackendError(format!("Failed to load scene: {}", e)))?;

        let scene_id = format!("scene_{}", uuid::Uuid::new_v4());
        let handle = UnrealSceneHandle {
            scene_id: scene_id.clone(),
            level_name: scene_path.to_string(),
        };

        self.scenes.insert(scene_id.clone(), handle);

        Ok(SceneHandle::new(scene_id, BackendType::UnrealEngine5))
    }

    fn get_scene_bounds(&self, scene: &SceneHandle) -> SimResult<(Position, Position)> {
        // Query UE5 for scene bounds
        // For now, return default bounds
        Ok((
            Point3::new(-1000.0, -1000.0, -100.0),
            Point3::new(1000.0, 1000.0, 500.0),
        ))
    }

    fn add_object(&mut self, scene: &SceneHandle, object: SceneObject) -> SimResult<String> {
        // Send spawn actor command to UE5
        Ok(object.id)
    }

    fn remove_object(&mut self, scene: &SceneHandle, object_id: &str) -> SimResult<()> {
        // Send destroy actor command to UE5
        Ok(())
    }

    fn update_transform(
        &mut self,
        scene: &SceneHandle,
        object_id: &str,
        transform: Transform,
    ) -> SimResult<()> {
        // Send set actor transform command to UE5
        Ok(())
    }

    fn cast_ray(&self, scene: &SceneHandle, ray: &Ray) -> SimResult<Option<RayHit>> {
        // Use UE5's line trace
        // This would be async in a real implementation
        Ok(None)
    }

    fn cast_rays(&self, scene: &SceneHandle, rays: &[Ray]) -> SimResult<Vec<Option<RayHit>>> {
        // Batch ray tracing in UE5
        Ok(vec![None; rays.len()])
    }

    fn get_objects(&self, scene: &SceneHandle) -> SimResult<Vec<SceneObject>> {
        // Query all actors in the level
        Ok(Vec::new())
    }

    async fn step(&mut self, delta_time: f64) -> SimResult<()> {
        self.time += delta_time;

        // Send tick command to UE5
        if let Some(conn) = &self.connection {
            let msg = UnrealMessage::Step { delta_time };
            conn.read()
                .await
                .send_message(msg)
                .await
                .map_err(|e| SimError::BackendError(format!("Step failed: {}", e)))?;
        }

        Ok(())
    }

    fn get_time(&self) -> f64 {
        self.time
    }

    async fn spawn_vehicle(&mut self, spec: VehicleSpec) -> SimResult<String> {
        // For single vehicle, just add to batch list - actual spawn happens in batch
        let handle = UnrealVehicleHandle {
            vehicle_id: spec.vehicle_id.clone(),
            pawn_name: format!("BP_{}_{}", spec.vehicle_type as u8, spec.vehicle_id),
            vehicle_type: spec.vehicle_type,
        };

        self.vehicles.insert(spec.vehicle_id.clone(), handle);

        Ok(spec.vehicle_id)
    }

    async fn remove_vehicle(&mut self, vehicle_id: &str) -> SimResult<()> {
        let conn = self
            .connection
            .as_ref()
            .ok_or_else(|| SimError::NotInitialized("Backend not initialized".to_string()))?;

        let msg = UnrealMessage::RemoveVehicle {
            vehicle_id: vehicle_id.to_string(),
        };

        conn.read()
            .await
            .send_message(msg)
            .await
            .map_err(|e| SimError::BackendError(format!("Failed to remove vehicle: {}", e)))?;

        self.vehicles.remove(vehicle_id);

        Ok(())
    }

    fn get_vehicle_state(&self, vehicle_id: &str) -> SimResult<VehicleState> {
        // Query UE5 for vehicle state
        // For now, return a default state
        let handle = self
            .vehicles
            .get(vehicle_id)
            .ok_or_else(|| SimError::BackendError(format!("Vehicle not found: {}", vehicle_id)))?;

        Ok(VehicleState {
            vehicle_id: vehicle_id.to_string(),
            timestamp: self.time,
            transform: Transform::identity(),
            linear_velocity: Vector3::zeros(),
            angular_velocity: Vector3::zeros(),
            linear_acceleration: Vector3::zeros(),
            angular_acceleration: Vector3::zeros(),
            battery_level: 1.0,
            is_grounded: false,
            collision_info: None,
        })
    }

    fn set_vehicle_control(&mut self, vehicle_id: &str, control: VehicleControl) -> SimResult<()> {
        // Send control inputs to UE5
        Ok(())
    }

    fn get_sensor_data(&self, vehicle_id: &str, sensor_id: &str) -> SimResult<SensorData> {
        // Query sensor data from UE5
        Err(SimError::BackendError(
            "Sensor data not yet implemented for UE5".to_string(),
        ))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_unreal_backend_creation() {
        let backend = UnrealEngine5Backend::new();
        assert_eq!(backend.name(), "Unreal Engine 5 Backend");
        assert!(!backend.is_initialized());
    }

    #[test]
    fn test_unreal_config() {
        let config = UnrealBackendConfig::default();
        assert_eq!(config.host, "127.0.0.1");
        assert_eq!(config.port, 41451);
    }
}
