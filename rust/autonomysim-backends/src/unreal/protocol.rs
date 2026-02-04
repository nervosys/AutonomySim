//! Message protocol for Unreal Engine 5 communication
//!
//! This module defines the JSON-RPC protocol between AutonomySim Rust backend
//! and Unreal Engine 5. The protocol supports:
//! - Robot spawning and destruction
//! - Batched position updates for high-performance swarm visualization
//! - Telemetry streaming (battery, health, comm status)
//! - Debug visualization (paths, connections, status indicators)
//! - Camera/sensor capture

use autonomysim_core::Transform;
use serde::{Deserialize, Serialize};

/// Robot type identifier for visual differentiation
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq)]
pub enum RobotType {
    Scout,       // Red
    Transport,   // Blue
    Combat,      // Green
    Relay,       // Yellow
    Coordinator, // Purple
}

impl RobotType {
    /// Get the color for this robot type in RGBA (0-255)
    pub fn color(&self) -> [u8; 4] {
        match self {
            RobotType::Scout => [255, 64, 64, 255],        // Red
            RobotType::Transport => [64, 128, 255, 255],   // Blue
            RobotType::Combat => [64, 255, 64, 255],       // Green
            RobotType::Relay => [255, 255, 64, 255],       // Yellow
            RobotType::Coordinator => [192, 64, 255, 255], // Purple
        }
    }

    /// Get the Unreal blueprint path for this robot type
    pub fn blueprint_path(&self) -> &'static str {
        match self {
            RobotType::Scout => "/Game/Robots/BP_ScoutDrone",
            RobotType::Transport => "/Game/Robots/BP_TransportUGV",
            RobotType::Combat => "/Game/Robots/BP_CombatDrone",
            RobotType::Relay => "/Game/Robots/BP_RelayDrone",
            RobotType::Coordinator => "/Game/Robots/BP_CoordinatorDrone",
        }
    }
}

/// Robot spawn data for batch spawning
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RobotSpawnData {
    pub id: i32,
    pub robot_type: RobotType,
    pub x: f64, // Unreal units (cm)
    pub y: f64,
    pub z: f64,
    pub yaw: f64, // Degrees
}

/// Robot position update for batched updates
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RobotPositionUpdate {
    pub id: i32,
    pub x: f64,
    pub y: f64,
    pub z: f64,
    pub yaw: f64,
    pub pitch: f64,
    pub roll: f64,
}

/// Robot telemetry data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RobotTelemetry {
    pub id: i32,
    pub battery_percent: f32,
    pub health_percent: f32,
    pub signal_strength_dbm: f32,
    pub is_jammed: bool,
    pub is_active: bool,
    pub current_task: Option<String>,
}

/// Debug line for visualization
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DebugLine {
    pub start: [f64; 3],
    pub end: [f64; 3],
    pub color: [u8; 4],
    pub thickness: f32,
    pub duration: f32, // 0 = single frame
}

/// Debug sphere for visualization
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DebugSphere {
    pub center: [f64; 3],
    pub radius: f64,
    pub color: [u8; 4],
    pub duration: f32,
}

/// Messages sent to Unreal Engine 5
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "type")]
pub enum UnrealMessage {
    // === Lifecycle Commands ===
    /// Load a level/map
    LoadLevel { level_name: String },

    /// Spawn a single vehicle (legacy)
    SpawnVehicle {
        vehicle_id: String,
        vehicle_type: String,
        transform: Transform,
    },

    /// Batch spawn multiple robots
    SpawnRobots { robots: Vec<RobotSpawnData> },

    /// Remove a vehicle
    RemoveVehicle { vehicle_id: String },

    /// Remove all robots
    ClearAllRobots,

    // === Simulation Control ===
    /// Step simulation
    Step { delta_time: f64 },

    /// Pause simulation
    Pause,

    /// Resume simulation
    Resume,

    /// Reset simulation to initial state
    Reset,

    // === Robot Control ===
    /// Set vehicle control inputs (legacy)
    SetControl {
        vehicle_id: String,
        throttle: f64,
        steering: f64,
        brake: f64,
        pitch: f64,
        roll: f64,
        yaw: f64,
    },

    /// Batch update robot positions (high-performance)
    UpdatePositions { positions: Vec<RobotPositionUpdate> },

    /// Update robot telemetry data
    UpdateTelemetry { telemetry: Vec<RobotTelemetry> },

    // === State Queries ===
    /// Get vehicle state
    GetState { vehicle_id: String },

    /// Get all robot states
    GetAllStates,

    // === Sensors ===
    /// Cast a ray
    CastRay {
        origin: [f64; 3],
        direction: [f64; 3],
        max_distance: f64,
    },

    /// Capture camera image
    CaptureImage {
        vehicle_id: String,
        camera_name: String,
        image_type: String,
    },

    // === Debug Visualization ===
    /// Draw debug lines (e.g., communication links)
    DrawDebugLines { lines: Vec<DebugLine> },

    /// Draw debug spheres (e.g., RF range indicators)
    DrawDebugSpheres { spheres: Vec<DebugSphere> },

    /// Clear all debug visualization
    ClearDebug,

    /// Set visualization mode
    SetVisualizationMode {
        show_comm_links: bool,
        show_rf_range: bool,
        show_paths: bool,
        show_labels: bool,
    },
}

/// Responses from Unreal Engine 5
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "status")]
pub enum UnrealResponse {
    /// Success response
    Success {
        request_id: String,
        data: serde_json::Value,
    },

    /// Error response
    Error {
        request_id: String,
        message: String,
        code: i32,
    },
}

/// RPC method names for JSON-RPC protocol
pub mod methods {
    pub const SPAWN_ROBOTS: &str = "spawn_robots";
    pub const UPDATE_POSITIONS: &str = "update_positions";
    pub const UPDATE_TELEMETRY: &str = "update_telemetry";
    pub const SIMULATION_STEP: &str = "simulation_step";
    pub const LOAD_LEVEL: &str = "load_level";
    pub const CLEAR_ALL_ROBOTS: &str = "clear_all_robots";
    pub const DRAW_DEBUG_LINES: &str = "draw_debug_lines";
    pub const DRAW_DEBUG_SPHERES: &str = "draw_debug_spheres";
    pub const CLEAR_DEBUG: &str = "clear_debug";
    pub const SET_VIS_MODE: &str = "set_visualization_mode";
    pub const GET_ALL_STATES: &str = "get_all_states";
    pub const PAUSE: &str = "pause";
    pub const RESUME: &str = "resume";
    pub const RESET: &str = "reset";
}
