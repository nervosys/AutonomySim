//! Connection management for Unreal Engine 5
//!
//! This module handles the TCP connection to the AutonomySimRPC plugin
//! running in Unreal Engine 5. It supports:
//! - Automatic reconnection on connection loss
//! - Batched message sending for efficiency
//! - Request/response correlation via JSON-RPC IDs

use crate::unreal::protocol::{
    methods, DebugLine, DebugSphere, RobotPositionUpdate, RobotSpawnData, RobotTelemetry,
    UnrealMessage, UnrealResponse,
};
use std::io;
use std::sync::atomic::{AtomicI32, Ordering};
use tokio::io::{AsyncBufReadExt, AsyncWriteExt, BufReader};
use tokio::net::TcpStream;
use tokio::sync::Mutex;
use tracing::{debug, info, warn};

/// Connection to Unreal Engine 5
pub struct UnrealConnection {
    stream: Mutex<TcpStream>,
    address: String,
    request_id: AtomicI32,
}

impl UnrealConnection {
    /// Connect to Unreal Engine 5
    pub async fn connect(host: &str, port: u16) -> io::Result<Self> {
        let address = format!("{}:{}", host, port);
        info!("Connecting to Unreal Engine at {}...", address);
        let stream = TcpStream::connect(&address).await?;
        info!("✓ Connected to Unreal Engine 5 at {}", address);

        Ok(Self {
            stream: Mutex::new(stream),
            address,
            request_id: AtomicI32::new(1),
        })
    }

    /// Send a JSON-RPC message and wait for response
    async fn send_rpc(
        &self,
        method: &str,
        params: serde_json::Value,
    ) -> io::Result<UnrealResponse> {
        let id = self.request_id.fetch_add(1, Ordering::SeqCst);

        let json_rpc = serde_json::json!({
            "jsonrpc": "2.0",
            "id": id,
            "method": method,
            "params": params
        });

        let mut json_str = serde_json::to_string(&json_rpc)
            .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))?;
        json_str.push('\n'); // Newline delimiter

        debug!("Sending RPC: {} (id={})", method, id);

        // Send message
        let mut stream = self.stream.lock().await;
        stream.write_all(json_str.as_bytes()).await?;
        stream.flush().await?;

        // Read response
        let mut reader = BufReader::new(&mut *stream);
        let mut response_line = String::new();
        reader.read_line(&mut response_line).await?;

        // Parse response
        if let Ok(json) = serde_json::from_str::<serde_json::Value>(&response_line) {
            if let Some(error) = json.get("error") {
                return Ok(UnrealResponse::Error {
                    request_id: id.to_string(),
                    message: error["message"]
                        .as_str()
                        .unwrap_or("Unknown error")
                        .to_string(),
                    code: error["code"].as_i64().unwrap_or(-1) as i32,
                });
            }
            return Ok(UnrealResponse::Success {
                request_id: id.to_string(),
                data: json
                    .get("result")
                    .cloned()
                    .unwrap_or(serde_json::Value::Null),
            });
        }

        Ok(UnrealResponse::Success {
            request_id: id.to_string(),
            data: serde_json::Value::Null,
        })
    }

    /// Send a message to UE5 and wait for response (legacy interface)
    pub async fn send_message(&self, message: UnrealMessage) -> io::Result<UnrealResponse> {
        let (method, params) = match &message {
            UnrealMessage::SpawnVehicle {
                vehicle_id,
                vehicle_type: _,
                transform,
            } => {
                let robots: Vec<serde_json::Value> = vec![serde_json::json!({
                    "id": vehicle_id.replace("robot_", "").parse::<i32>().unwrap_or(0),
                    "x": transform.position.x * 100.0, // Convert to Unreal units (cm)
                    "y": transform.position.y * 100.0,
                    "z": transform.position.z * 100.0,
                    "yaw": 0.0
                })];
                (
                    methods::SPAWN_ROBOTS,
                    serde_json::json!({ "robots": robots }),
                )
            }
            UnrealMessage::Step { delta_time } => (
                methods::SIMULATION_STEP,
                serde_json::json!({ "delta_time": delta_time }),
            ),
            UnrealMessage::LoadLevel { level_name } => (
                methods::LOAD_LEVEL,
                serde_json::json!({ "level": level_name }),
            ),
            UnrealMessage::SpawnRobots { robots } => (
                methods::SPAWN_ROBOTS,
                serde_json::json!({ "robots": robots }),
            ),
            UnrealMessage::UpdatePositions { positions } => (
                methods::UPDATE_POSITIONS,
                serde_json::json!({ "positions": positions }),
            ),
            UnrealMessage::UpdateTelemetry { telemetry } => (
                methods::UPDATE_TELEMETRY,
                serde_json::json!({ "telemetry": telemetry }),
            ),
            UnrealMessage::ClearAllRobots => (methods::CLEAR_ALL_ROBOTS, serde_json::json!({})),
            UnrealMessage::DrawDebugLines { lines } => (
                methods::DRAW_DEBUG_LINES,
                serde_json::json!({ "lines": lines }),
            ),
            UnrealMessage::DrawDebugSpheres { spheres } => (
                methods::DRAW_DEBUG_SPHERES,
                serde_json::json!({ "spheres": spheres }),
            ),
            UnrealMessage::ClearDebug => (methods::CLEAR_DEBUG, serde_json::json!({})),
            UnrealMessage::SetVisualizationMode {
                show_comm_links,
                show_rf_range,
                show_paths,
                show_labels,
            } => (
                methods::SET_VIS_MODE,
                serde_json::json!({
                    "show_comm_links": show_comm_links,
                    "show_rf_range": show_rf_range,
                    "show_paths": show_paths,
                    "show_labels": show_labels
                }),
            ),
            UnrealMessage::Pause => (methods::PAUSE, serde_json::json!({})),
            UnrealMessage::Resume => (methods::RESUME, serde_json::json!({})),
            UnrealMessage::Reset => (methods::RESET, serde_json::json!({})),
            UnrealMessage::GetAllStates => (methods::GET_ALL_STATES, serde_json::json!({})),
            UnrealMessage::SetFpvCamera {
                vehicle_id,
                tilt_angle_deg,
                fov_h_deg,
                resolution_width,
                resolution_height,
                lens_distortion,
                latency_ms,
            } => (
                methods::SET_FPV_CAMERA,
                serde_json::json!({
                    "vehicle_id": vehicle_id,
                    "tilt_angle_deg": tilt_angle_deg,
                    "fov_h_deg": fov_h_deg,
                    "resolution_width": resolution_width,
                    "resolution_height": resolution_height,
                    "lens_distortion": lens_distortion,
                    "latency_ms": latency_ms
                }),
            ),
            UnrealMessage::SetFpvControl {
                vehicle_id,
                throttle,
                roll,
                pitch,
                yaw,
                flight_mode,
            } => (
                methods::SET_FPV_CONTROL,
                serde_json::json!({
                    "vehicle_id": vehicle_id,
                    "throttle": throttle,
                    "roll": roll,
                    "pitch": pitch,
                    "yaw": yaw,
                    "flight_mode": flight_mode
                }),
            ),
            UnrealMessage::ArmDrone { vehicle_id, armed } => (
                methods::ARM_DRONE,
                serde_json::json!({
                    "vehicle_id": vehicle_id,
                    "armed": armed
                }),
            ),
            UnrealMessage::SpawnFpvDrone {
                vehicle_id,
                drone_preset,
                x,
                y,
                z,
                yaw,
            } => (
                methods::SPAWN_FPV_DRONE,
                serde_json::json!({
                    "vehicle_id": vehicle_id,
                    "drone_preset": drone_preset,
                    "x": x, "y": y, "z": z, "yaw": yaw
                }),
            ),
            UnrealMessage::UpdateFpvState {
                vehicle_id,
                x,
                y,
                z,
                qw,
                qx,
                qy,
                qz,
                speed_mps,
                altitude_m,
                motor_outputs,
                battery_voltage,
                battery_remaining,
                flight_mode,
                armed,
                osd,
            } => (
                methods::UPDATE_FPV_STATE,
                serde_json::json!({
                    "vehicle_id": vehicle_id,
                    "position": [x * 100.0, y * 100.0, z * 100.0],
                    "orientation": [qx, qy, qz, qw],
                    "speed_mps": speed_mps,
                    "altitude_m": altitude_m,
                    "motor_outputs": motor_outputs,
                    "battery_voltage": battery_voltage,
                    "battery_remaining": battery_remaining,
                    "flight_mode": flight_mode,
                    "armed": armed,
                    "osd": osd
                }),
            ),
            UnrealMessage::SetOsdVisible {
                vehicle_id,
                visible,
            } => (
                methods::SET_OSD_VISIBLE,
                serde_json::json!({
                    "vehicle_id": vehicle_id,
                    "visible": visible
                }),
            ),
            // Remaining variants that don't have dedicated UE5 handlers yet.
            // These are explicitly listed (not `_ =>`) so adding a new variant
            // to UnrealMessage produces a compile error, forcing the developer
            // to add the handler here.
            UnrealMessage::RemoveVehicle { vehicle_id } => {
                warn!(
                    "RemoveVehicle not implemented in UE5 server (vehicle: {})",
                    vehicle_id
                );
                ("ping", serde_json::json!({}))
            }
            UnrealMessage::SetControl { vehicle_id, .. } => {
                warn!(
                    "SetControl not implemented in UE5 server (vehicle: {})",
                    vehicle_id
                );
                ("ping", serde_json::json!({}))
            }
            UnrealMessage::GetState { vehicle_id } => {
                warn!(
                    "GetState not implemented in UE5 server (vehicle: {})",
                    vehicle_id
                );
                ("ping", serde_json::json!({}))
            }
            UnrealMessage::CastRay { .. } => {
                warn!("CastRay not implemented in UE5 server");
                ("ping", serde_json::json!({}))
            }
            UnrealMessage::CaptureImage { vehicle_id, .. } => {
                warn!(
                    "CaptureImage not implemented in UE5 server (vehicle: {})",
                    vehicle_id
                );
                ("ping", serde_json::json!({}))
            }
        };

        self.send_rpc(method, params).await
    }

    /// Batch spawn multiple robots in a single RPC call (legacy interface)
    pub async fn spawn_robots_batch(
        &self,
        robots: Vec<(i32, f64, f64, f64, f64)>,
    ) -> io::Result<UnrealResponse> {
        let robot_data: Vec<serde_json::Value> = robots
            .iter()
            .map(|(robot_id, x, y, z, yaw)| {
                serde_json::json!({
                    "id": robot_id,
                    "x": x * 100.0, // Convert to Unreal units (cm)
                    "y": y * 100.0,
                    "z": z * 100.0 + 100.0, // Add some height offset
                    "yaw": yaw
                })
            })
            .collect();

        self.send_rpc(
            methods::SPAWN_ROBOTS,
            serde_json::json!({ "robots": robot_data }),
        )
        .await
    }

    /// Spawn robots with full type information
    pub async fn spawn_robots(&self, robots: Vec<RobotSpawnData>) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SPAWN_ROBOTS,
            serde_json::json!({ "robots": robots }),
        )
        .await
    }

    /// Batch update robot positions (high performance)
    pub async fn update_positions(
        &self,
        positions: Vec<RobotPositionUpdate>,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::UPDATE_POSITIONS,
            serde_json::json!({ "positions": positions }),
        )
        .await
    }

    /// Update robot telemetry
    pub async fn update_telemetry(
        &self,
        telemetry: Vec<RobotTelemetry>,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::UPDATE_TELEMETRY,
            serde_json::json!({ "telemetry": telemetry }),
        )
        .await
    }

    /// Draw debug lines for communication visualization
    pub async fn draw_debug_lines(&self, lines: Vec<DebugLine>) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::DRAW_DEBUG_LINES,
            serde_json::json!({ "lines": lines }),
        )
        .await
    }

    /// Draw debug spheres for RF range visualization
    pub async fn draw_debug_spheres(
        &self,
        spheres: Vec<DebugSphere>,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::DRAW_DEBUG_SPHERES,
            serde_json::json!({ "spheres": spheres }),
        )
        .await
    }

    /// Clear all debug visualization
    pub async fn clear_debug(&self) -> io::Result<UnrealResponse> {
        self.send_rpc(methods::CLEAR_DEBUG, serde_json::json!({}))
            .await
    }

    /// Set visualization mode
    pub async fn set_visualization_mode(
        &self,
        show_comm_links: bool,
        show_rf_range: bool,
        show_paths: bool,
        show_labels: bool,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SET_VIS_MODE,
            serde_json::json!({
                "show_comm_links": show_comm_links,
                "show_rf_range": show_rf_range,
                "show_paths": show_paths,
                "show_labels": show_labels
            }),
        )
        .await
    }

    /// Clear all robots from the scene
    pub async fn clear_all_robots(&self) -> io::Result<UnrealResponse> {
        self.send_rpc(methods::CLEAR_ALL_ROBOTS, serde_json::json!({}))
            .await
    }

    /// Step the simulation
    pub async fn step(&self, delta_time: f64) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SIMULATION_STEP,
            serde_json::json!({ "delta_time": delta_time }),
        )
        .await
    }

    /// Disconnect from UE5
    pub async fn disconnect(&mut self) -> io::Result<()> {
        info!("Disconnecting from Unreal Engine...");
        self.stream.lock().await.shutdown().await
    }

    // ─── FPV Methods ─────────────────────────────────────────────────────────

    /// Spawn an FPV racing drone
    pub async fn spawn_fpv_drone(
        &self,
        vehicle_id: &str,
        drone_preset: &str,
        x: f64,
        y: f64,
        z: f64,
        yaw: f64,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SPAWN_FPV_DRONE,
            serde_json::json!({
                "vehicle_id": vehicle_id,
                "drone_preset": drone_preset,
                "x": x * 100.0,
                "y": y * 100.0,
                "z": z * 100.0,
                "yaw": yaw
            }),
        )
        .await
    }

    /// Configure FPV camera on a drone
    pub async fn set_fpv_camera(
        &self,
        vehicle_id: &str,
        tilt_angle_deg: f64,
        fov_h_deg: f64,
        resolution_width: u32,
        resolution_height: u32,
        lens_distortion: bool,
        latency_ms: f64,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SET_FPV_CAMERA,
            serde_json::json!({
                "vehicle_id": vehicle_id,
                "tilt_angle_deg": tilt_angle_deg,
                "fov_h_deg": fov_h_deg,
                "resolution_width": resolution_width,
                "resolution_height": resolution_height,
                "lens_distortion": lens_distortion,
                "latency_ms": latency_ms
            }),
        )
        .await
    }

    /// Send FPV stick input
    pub async fn set_fpv_control(
        &self,
        vehicle_id: &str,
        throttle: f64,
        roll: f64,
        pitch: f64,
        yaw: f64,
        flight_mode: &str,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SET_FPV_CONTROL,
            serde_json::json!({
                "vehicle_id": vehicle_id,
                "throttle": throttle,
                "roll": roll,
                "pitch": pitch,
                "yaw": yaw,
                "flight_mode": flight_mode
            }),
        )
        .await
    }

    /// Arm or disarm an FPV drone
    pub async fn arm_drone(&self, vehicle_id: &str, armed: bool) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::ARM_DRONE,
            serde_json::json!({
                "vehicle_id": vehicle_id,
                "armed": armed
            }),
        )
        .await
    }

    /// Send FPV drone state update to UE5 (position, orientation, OSD)
    pub async fn update_fpv_state(
        &self,
        state: &autonomysim_core::fpv::FpvState,
    ) -> io::Result<UnrealResponse> {
        use crate::unreal::protocol::FpvOsdData;

        let osd_data: FpvOsdData = state.osd.clone().into();
        self.send_rpc(
            methods::UPDATE_FPV_STATE,
            serde_json::json!({
                "vehicle_id": state.vehicle_id,
                "x": state.position.x * 100.0,
                "y": state.position.y * 100.0,
                "z": state.position.z * 100.0,
                "qw": state.orientation.quaternion().w,
                "qx": state.orientation.quaternion().i,
                "qy": state.orientation.quaternion().j,
                "qz": state.orientation.quaternion().k,
                "speed_mps": state.speed_mps,
                "altitude_m": state.altitude_m,
                "motor_outputs": state.motor_outputs,
                "battery_voltage": state.battery_voltage,
                "battery_remaining": state.battery_remaining,
                "flight_mode": format!("{:?}", state.flight_mode).to_lowercase(),
                "armed": state.armed,
                "osd": osd_data
            }),
        )
        .await
    }

    /// Set OSD visibility
    pub async fn set_osd_visible(
        &self,
        vehicle_id: &str,
        visible: bool,
    ) -> io::Result<UnrealResponse> {
        self.send_rpc(
            methods::SET_OSD_VISIBLE,
            serde_json::json!({
                "vehicle_id": vehicle_id,
                "visible": visible
            }),
        )
        .await
    }
}
