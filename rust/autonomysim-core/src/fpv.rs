//! First Person View (FPV) drone racing support
//!
//! Provides FPV flight modes similar to TRYP FPV and Liftoff drone racing simulators:
//! - Rate/Acro mode (direct angular rate control)
//! - Angle/Stabilize mode (self-leveling)
//! - Horizon mode (hybrid: acro center, self-level at extremes)
//! - Camera tilt control with configurable angle
//! - OSD (On Screen Display) telemetry
//! - Analog-like stick input with expo curves

use crate::backend::{Position, Rotation, Transform, Vec3};
use serde::{Deserialize, Serialize};

// ─── Flight Mode ─────────────────────────────────────────────────────────────

/// FPV flight mode (selectable in-flight like real FC firmware)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum FpvFlightMode {
    /// Direct angular rate control — no self-leveling.
    /// Stick input maps directly to roll/pitch/yaw rates (deg/s).
    /// This is the default mode for racing and freestyle.
    Acro,

    /// Self-leveling — stick input maps to target angle.
    /// Releasing the stick returns the drone to level.
    Angle,

    /// Hybrid — behaves like Angle near center, Acro at extremes.
    /// Good for learning FPV.
    Horizon,
}

impl Default for FpvFlightMode {
    fn default() -> Self {
        FpvFlightMode::Acro
    }
}

// ─── Rates Profile ───────────────────────────────────────────────────────────

/// Rate profile defining max angular rates (like Betaflight rates)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RatesProfile {
    /// Name of this profile (e.g., "Race", "Freestyle", "Cinematic")
    pub name: String,

    /// Roll rate: center sensitivity (deg/s)
    pub roll_rc_rate: f64,
    /// Roll rate: max rate (deg/s)
    pub roll_super_rate: f64,
    /// Roll expo (0.0 = linear, 1.0 = max expo curve)
    pub roll_expo: f64,

    /// Pitch rate: center sensitivity (deg/s)
    pub pitch_rc_rate: f64,
    /// Pitch rate: max rate (deg/s)
    pub pitch_super_rate: f64,
    /// Pitch expo (0.0 = linear, 1.0 = max expo)
    pub pitch_expo: f64,

    /// Yaw rate: center sensitivity (deg/s)
    pub yaw_rc_rate: f64,
    /// Yaw rate: max rate (deg/s)
    pub yaw_super_rate: f64,
    /// Yaw expo (0.0 = linear, 1.0 = max expo)
    pub yaw_expo: f64,

    /// Throttle mid-point (0.0–1.0, default 0.5)
    pub throttle_mid: f64,
    /// Throttle expo (0.0 = linear, 1.0 = max expo)
    pub throttle_expo: f64,
}

impl Default for RatesProfile {
    fn default() -> Self {
        Self::race()
    }
}

impl RatesProfile {
    /// Racing rates — high rates, moderate expo
    pub fn race() -> Self {
        Self {
            name: "Race".to_string(),
            roll_rc_rate: 200.0,
            roll_super_rate: 700.0,
            roll_expo: 0.3,
            pitch_rc_rate: 200.0,
            pitch_super_rate: 700.0,
            pitch_expo: 0.3,
            yaw_rc_rate: 180.0,
            yaw_super_rate: 500.0,
            yaw_expo: 0.2,
            throttle_mid: 0.5,
            throttle_expo: 0.2,
        }
    }

    /// Freestyle rates — very high rates, more expo
    pub fn freestyle() -> Self {
        Self {
            name: "Freestyle".to_string(),
            roll_rc_rate: 200.0,
            roll_super_rate: 850.0,
            roll_expo: 0.5,
            pitch_rc_rate: 200.0,
            pitch_super_rate: 850.0,
            pitch_expo: 0.5,
            yaw_rc_rate: 180.0,
            yaw_super_rate: 600.0,
            yaw_expo: 0.4,
            throttle_mid: 0.5,
            throttle_expo: 0.3,
        }
    }

    /// Cinematic rates — low rates, smooth control
    pub fn cinematic() -> Self {
        Self {
            name: "Cinematic".to_string(),
            roll_rc_rate: 100.0,
            roll_super_rate: 300.0,
            roll_expo: 0.7,
            pitch_rc_rate: 100.0,
            pitch_super_rate: 300.0,
            pitch_expo: 0.7,
            yaw_rc_rate: 80.0,
            yaw_super_rate: 200.0,
            yaw_expo: 0.6,
            throttle_mid: 0.5,
            throttle_expo: 0.5,
        }
    }

    /// Apply Betaflight-style expo curve to a stick input
    /// Input: stick value in [-1.0, 1.0]
    /// Returns: modified stick value with expo applied
    pub fn apply_expo(stick: f64, expo: f64) -> f64 {
        let s = stick.clamp(-1.0, 1.0);
        let abs_s = s.abs();
        // Betaflight expo formula: s * (1 - expo + expo * s^2)
        s * (1.0 - expo + expo * abs_s * abs_s)
    }

    /// Compute the target roll rate (deg/s) from stick input [-1.0, 1.0]
    pub fn roll_rate(&self, stick: f64) -> f64 {
        let expo_stick = Self::apply_expo(stick, self.roll_expo);
        let base = self.roll_rc_rate * expo_stick;
        let super_factor =
            1.0 / (1.0 - (expo_stick.abs() * self.roll_super_rate / 1000.0)).max(0.01);
        base * super_factor
    }

    /// Compute the target pitch rate (deg/s) from stick input [-1.0, 1.0]
    pub fn pitch_rate(&self, stick: f64) -> f64 {
        let expo_stick = Self::apply_expo(stick, self.pitch_expo);
        let base = self.pitch_rc_rate * expo_stick;
        let super_factor =
            1.0 / (1.0 - (expo_stick.abs() * self.pitch_super_rate / 1000.0)).max(0.01);
        base * super_factor
    }

    /// Compute the target yaw rate (deg/s) from stick input [-1.0, 1.0]
    pub fn yaw_rate(&self, stick: f64) -> f64 {
        let expo_stick = Self::apply_expo(stick, self.yaw_expo);
        let base = self.yaw_rc_rate * expo_stick;
        let super_factor =
            1.0 / (1.0 - (expo_stick.abs() * self.yaw_super_rate / 1000.0)).max(0.01);
        base * super_factor
    }

    /// Apply throttle expo (input 0.0–1.0, output 0.0–1.0)
    pub fn throttle_curve(&self, stick: f64) -> f64 {
        let s = stick.clamp(0.0, 1.0);
        let centered = (s - self.throttle_mid) / (1.0 - self.throttle_mid).max(0.01);
        let expo_centered = Self::apply_expo(centered, self.throttle_expo);
        (self.throttle_mid + expo_centered * (1.0 - self.throttle_mid)).clamp(0.0, 1.0)
    }

    /// Approximate inverse of throttle_curve: given desired output, find the input
    pub fn throttle_curve_inv(&self, target: f64) -> f64 {
        // Binary search for the input that produces the target output
        let mut lo = 0.0_f64;
        let mut hi = 1.0_f64;
        for _ in 0..32 {
            let mid = (lo + hi) / 2.0;
            if self.throttle_curve(mid) < target {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        (lo + hi) / 2.0
    }
}

// ─── FPV Camera ──────────────────────────────────────────────────────────────

/// FPV camera configuration (the "GoPro on a tilted mount")
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FpvCameraConfig {
    /// Camera tilt angle in degrees (positive = tilted up from horizontal).
    /// Typical racing: 25–45°. Freestyle: 20–35°. Cinematic: 5–15°.
    pub tilt_angle_deg: f64,

    /// Horizontal field of view in degrees.
    /// Wide-angle FPV cameras are typically 120–170°.
    pub fov_h_deg: f64,

    /// Aspect ratio (width / height). Default 16:9 = 1.778.
    pub aspect_ratio: f64,

    /// Resolution width in pixels
    pub resolution_width: u32,

    /// Resolution height in pixels
    pub resolution_height: u32,

    /// Camera latency in milliseconds (simulates video link delay)
    pub latency_ms: f64,

    /// Whether to apply lens distortion (barrel distortion)
    pub lens_distortion: bool,

    /// Camera position offset relative to drone center of mass (meters)
    pub offset: Vec3,
}

impl Default for FpvCameraConfig {
    fn default() -> Self {
        Self::racing()
    }
}

impl FpvCameraConfig {
    /// Racing camera: high tilt, very wide FOV
    pub fn racing() -> Self {
        Self {
            tilt_angle_deg: 35.0,
            fov_h_deg: 155.0,
            aspect_ratio: 16.0 / 9.0,
            resolution_width: 1280,
            resolution_height: 720,
            latency_ms: 20.0,
            lens_distortion: true,
            offset: Vec3::new(0.0, 0.02, 0.01), // Slightly forward and up
        }
    }

    /// Freestyle camera: moderate tilt, wide FOV
    pub fn freestyle() -> Self {
        Self {
            tilt_angle_deg: 25.0,
            fov_h_deg: 140.0,
            aspect_ratio: 16.0 / 9.0,
            resolution_width: 1920,
            resolution_height: 1080,
            latency_ms: 25.0,
            lens_distortion: true,
            offset: Vec3::new(0.0, 0.02, 0.01),
        }
    }

    /// Cinematic camera: low tilt, narrower FOV
    pub fn cinematic() -> Self {
        Self {
            tilt_angle_deg: 10.0,
            fov_h_deg: 110.0,
            aspect_ratio: 16.0 / 9.0,
            resolution_width: 3840,
            resolution_height: 2160,
            latency_ms: 30.0,
            lens_distortion: false,
            offset: Vec3::new(0.0, 0.02, 0.01),
        }
    }

    /// Vertical field of view in degrees
    pub fn fov_v_deg(&self) -> f64 {
        let fov_h_rad = self.fov_h_deg.to_radians();
        let fov_v_rad = 2.0 * (fov_h_rad / 2.0).tan().atan2(self.aspect_ratio);
        fov_v_rad.to_degrees()
    }
}

// ─── Drone Physics Config ────────────────────────────────────────────────────

/// FPV racing drone physical configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FpvDroneConfig {
    /// Drone name
    pub name: String,

    /// Frame size (motor-to-motor diagonal) in mm (e.g., 5" = 220mm)
    pub frame_size_mm: f64,

    /// All-up weight in grams
    pub weight_grams: f64,

    /// Max motor thrust per motor in grams
    pub max_thrust_per_motor_grams: f64,

    /// Number of motors (4 = quad, 6 = hex)
    pub motor_count: u32,

    /// Motor KV rating
    pub motor_kv: u32,

    /// Propeller size (inches)
    pub prop_size_inches: f64,

    /// Battery cell count (e.g., 4S, 6S)
    pub battery_cells: u32,

    /// Battery capacity in mAh
    pub battery_capacity_mah: u32,

    /// Flight controller loop rate in kHz (e.g., 8 for 8kHz)
    pub fc_loop_rate_khz: f64,

    /// PID gains (roll)
    pub pid_roll: PidGains,
    /// PID gains (pitch)
    pub pid_pitch: PidGains,
    /// PID gains (yaw)
    pub pid_yaw: PidGains,

    /// Maximum angle in Angle/Horizon mode (degrees)
    pub max_angle_deg: f64,

    /// Camera configuration
    pub camera: FpvCameraConfig,

    /// Rates profile
    pub rates: RatesProfile,

    /// Default flight mode
    pub default_mode: FpvFlightMode,

    /// Linear drag coefficient
    pub drag_coefficient: f64,

    /// Moment of inertia (kg·m²)
    pub inertia: Vec3,
}

impl Default for FpvDroneConfig {
    fn default() -> Self {
        Self::five_inch_race()
    }
}

impl FpvDroneConfig {
    /// 5" racing quad — the standard FPV racing platform
    pub fn five_inch_race() -> Self {
        Self {
            name: "5\" Race Quad".to_string(),
            frame_size_mm: 220.0,
            weight_grams: 650.0,
            max_thrust_per_motor_grams: 1200.0,
            motor_count: 4,
            motor_kv: 2400,
            prop_size_inches: 5.0,
            battery_cells: 6,
            battery_capacity_mah: 1300,
            fc_loop_rate_khz: 8.0,
            pid_roll: PidGains::default_roll(),
            pid_pitch: PidGains::default_pitch(),
            pid_yaw: PidGains::default_yaw(),
            max_angle_deg: 55.0,
            camera: FpvCameraConfig::racing(),
            rates: RatesProfile::race(),
            default_mode: FpvFlightMode::Acro,
            drag_coefficient: 0.25,
            inertia: Vec3::new(0.003, 0.003, 0.005),
        }
    }

    /// 5" freestyle quad — heavier, more battery, lower KV
    pub fn five_inch_freestyle() -> Self {
        Self {
            name: "5\" Freestyle Quad".to_string(),
            frame_size_mm: 230.0,
            weight_grams: 750.0,
            max_thrust_per_motor_grams: 1400.0,
            motor_count: 4,
            motor_kv: 1900,
            prop_size_inches: 5.0,
            battery_cells: 6,
            battery_capacity_mah: 1500,
            fc_loop_rate_khz: 8.0,
            pid_roll: PidGains::default_roll(),
            pid_pitch: PidGains::default_pitch(),
            pid_yaw: PidGains::default_yaw(),
            max_angle_deg: 55.0,
            camera: FpvCameraConfig::freestyle(),
            rates: RatesProfile::freestyle(),
            default_mode: FpvFlightMode::Acro,
            drag_coefficient: 0.30,
            inertia: Vec3::new(0.004, 0.004, 0.006),
        }
    }

    /// 3" micro quad — indoor/proximity flying
    pub fn three_inch_micro() -> Self {
        Self {
            name: "3\" Micro Quad".to_string(),
            frame_size_mm: 140.0,
            weight_grams: 200.0,
            max_thrust_per_motor_grams: 400.0,
            motor_count: 4,
            motor_kv: 3600,
            prop_size_inches: 3.0,
            battery_cells: 4,
            battery_capacity_mah: 650,
            fc_loop_rate_khz: 8.0,
            pid_roll: PidGains {
                p: 55.0,
                i: 75.0,
                d: 30.0,
                f: 100.0,
            },
            pid_pitch: PidGains {
                p: 58.0,
                i: 78.0,
                d: 32.0,
                f: 105.0,
            },
            pid_yaw: PidGains {
                p: 60.0,
                i: 80.0,
                d: 0.0,
                f: 90.0,
            },
            max_angle_deg: 55.0,
            camera: FpvCameraConfig::racing(),
            rates: RatesProfile::race(),
            default_mode: FpvFlightMode::Acro,
            drag_coefficient: 0.20,
            inertia: Vec3::new(0.0008, 0.0008, 0.0015),
        }
    }

    /// 7" long-range quad — cinematic / long-distance
    pub fn seven_inch_longrange() -> Self {
        Self {
            name: "7\" Long Range Quad".to_string(),
            frame_size_mm: 295.0,
            weight_grams: 900.0,
            max_thrust_per_motor_grams: 1600.0,
            motor_count: 4,
            motor_kv: 1500,
            prop_size_inches: 7.0,
            battery_cells: 6,
            battery_capacity_mah: 2200,
            fc_loop_rate_khz: 4.0,
            pid_roll: PidGains::default_roll(),
            pid_pitch: PidGains::default_pitch(),
            pid_yaw: PidGains::default_yaw(),
            max_angle_deg: 45.0,
            camera: FpvCameraConfig::cinematic(),
            rates: RatesProfile::cinematic(),
            default_mode: FpvFlightMode::Angle,
            drag_coefficient: 0.35,
            inertia: Vec3::new(0.006, 0.006, 0.010),
        }
    }

    /// Thrust-to-weight ratio
    pub fn thrust_to_weight(&self) -> f64 {
        let total_thrust = self.max_thrust_per_motor_grams as f64 * self.motor_count as f64;
        total_thrust / self.weight_grams as f64
    }

    /// Mass in kg
    pub fn mass_kg(&self) -> f64 {
        self.weight_grams as f64 / 1000.0
    }

    /// Max total thrust in Newtons
    pub fn max_thrust_n(&self) -> f64 {
        let total_grams = self.max_thrust_per_motor_grams as f64 * self.motor_count as f64;
        total_grams / 1000.0 * 9.81
    }

    /// Compute the stick input needed for hover (accounting for throttle curve)
    pub fn hover_throttle_input(&self) -> f64 {
        let hover_fraction = self.mass_kg() * 9.81 / self.max_thrust_n();
        self.rates.throttle_curve_inv(hover_fraction)
    }
}

// ─── PID Gains ───────────────────────────────────────────────────────────────

/// PID controller gains (Betaflight-style values)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PidGains {
    /// Proportional gain
    pub p: f64,
    /// Integral gain
    pub i: f64,
    /// Derivative gain
    pub d: f64,
    /// Feed-forward gain
    pub f: f64,
}

impl PidGains {
    pub fn default_roll() -> Self {
        Self {
            p: 45.0,
            i: 70.0,
            d: 28.0,
            f: 120.0,
        }
    }

    pub fn default_pitch() -> Self {
        Self {
            p: 47.0,
            i: 72.0,
            d: 30.0,
            f: 125.0,
        }
    }

    pub fn default_yaw() -> Self {
        Self {
            p: 55.0,
            i: 80.0,
            d: 0.0,
            f: 100.0,
        }
    }
}

// ─── FPV Stick Input ─────────────────────────────────────────────────────────

/// Raw stick inputs from transmitter / gamepad / keyboard
/// All stick values are in [-1.0, 1.0] except throttle which is [0.0, 1.0]
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct FpvStickInput {
    /// Throttle (0.0 = idle, 1.0 = full power)
    pub throttle: f64,
    /// Roll stick (-1.0 = left, 1.0 = right)
    pub roll: f64,
    /// Pitch stick (-1.0 = forward/nose down, 1.0 = backward/nose up)
    pub pitch: f64,
    /// Yaw stick (-1.0 = left, 1.0 = right)
    pub yaw: f64,
}

impl FpvStickInput {
    /// Create from throttle, roll, pitch, yaw
    pub fn new(throttle: f64, roll: f64, pitch: f64, yaw: f64) -> Self {
        Self {
            throttle: throttle.clamp(0.0, 1.0),
            roll: roll.clamp(-1.0, 1.0),
            pitch: pitch.clamp(-1.0, 1.0),
            yaw: yaw.clamp(-1.0, 1.0),
        }
    }

    /// Hovering / neutral sticks
    pub fn hover() -> Self {
        Self {
            throttle: 0.5,
            roll: 0.0,
            pitch: 0.0,
            yaw: 0.0,
        }
    }
}

// ─── OSD (On Screen Display) ─────────────────────────────────────────────────

/// OSD telemetry overlay — data displayed on the FPV feed like a real OSD chip
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FpvOsd {
    /// Battery voltage in volts
    pub battery_voltage: f64,
    /// Current draw in amps
    pub current_amps: f64,
    /// mAh consumed
    pub mah_consumed: f64,
    /// RSSI (0–100)
    pub rssi: u8,
    /// Flight mode label
    pub flight_mode: String,
    /// Altitude in meters (relative to arm point)
    pub altitude_m: f64,
    /// Ground speed in m/s
    pub ground_speed_mps: f64,
    /// Flight time in seconds
    pub flight_time_s: f64,
    /// GPS satellite count
    pub gps_sats: u8,
    /// Throttle percentage (0–100)
    pub throttle_pct: u8,
    /// Craft name
    pub craft_name: String,
    /// Warnings (e.g., "LOW BATTERY", "FAILSAFE")
    pub warnings: Vec<String>,
    /// Crosshair / aim point visible
    pub show_crosshair: bool,
}

impl Default for FpvOsd {
    fn default() -> Self {
        Self {
            battery_voltage: 25.2, // 6S fully charged
            current_amps: 0.0,
            mah_consumed: 0.0,
            rssi: 99,
            flight_mode: "ACRO".to_string(),
            altitude_m: 0.0,
            ground_speed_mps: 0.0,
            flight_time_s: 0.0,
            gps_sats: 12,
            throttle_pct: 0,
            craft_name: "AUTONOMYSIM".to_string(),
            warnings: Vec::new(),
            show_crosshair: true,
        }
    }
}

// ─── FPV State ───────────────────────────────────────────────────────────────

/// Complete FPV drone state (updated every physics tick)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FpvState {
    /// Current vehicle ID
    pub vehicle_id: String,

    /// Current position (world space, meters)
    pub position: Position,

    /// Current orientation (world space)
    pub orientation: Rotation,

    /// Linear velocity (m/s, world frame)
    pub velocity: Vec3,

    /// Angular velocity (rad/s, body frame)
    pub angular_velocity: Vec3,

    /// Current motor RPMs (one per motor)
    pub motor_rpms: Vec<f64>,

    /// Current motor outputs (0.0–1.0, one per motor)
    pub motor_outputs: Vec<f64>,

    /// Battery remaining (0.0–1.0)
    pub battery_remaining: f64,

    /// Battery voltage
    pub battery_voltage: f64,

    /// Current flight mode
    pub flight_mode: FpvFlightMode,

    /// Is the drone armed?
    pub armed: bool,

    /// OSD telemetry
    pub osd: FpvOsd,

    /// Simulation timestamp
    pub timestamp: f64,

    /// Speed in m/s (magnitude of velocity)
    pub speed_mps: f64,

    /// Altitude above start point (meters)
    pub altitude_m: f64,
}

// ─── FPV Physics Sim ─────────────────────────────────────────────────────────

/// Simplified FPV physics simulation
/// Runs at high frequency (1kHz+) for realistic feel
pub struct FpvPhysics {
    pub config: FpvDroneConfig,
    pub position: Position,
    pub velocity: Vec3,
    pub orientation: Rotation,
    pub angular_velocity: Vec3,
    pub motor_outputs: Vec<f64>,
    pub armed: bool,
    pub flight_mode: FpvFlightMode,
    pub battery_voltage: f64,
    pub battery_remaining: f64,
    pub flight_time: f64,
    pub mah_consumed: f64,
    start_altitude: f64,
}

impl FpvPhysics {
    /// Create a new FPV physics sim at the given spawn position
    pub fn new(config: FpvDroneConfig, spawn: Position) -> Self {
        let mode = config.default_mode;
        let voltage = config.battery_cells as f64 * 4.2; // Fully charged LiPo
        let motor_count = config.motor_count as usize;
        let start_alt = spawn.z;
        Self {
            config,
            position: spawn,
            velocity: Vec3::zeros(),
            orientation: Rotation::identity(),
            angular_velocity: Vec3::zeros(),
            motor_outputs: vec![0.0; motor_count],
            armed: false,
            flight_mode: mode,
            battery_voltage: voltage,
            battery_remaining: 1.0,
            flight_time: 0.0,
            mah_consumed: 0.0,
            start_altitude: start_alt,
        }
    }

    /// Arm / disarm the drone
    pub fn set_armed(&mut self, armed: bool) {
        self.armed = armed;
        if !armed {
            self.motor_outputs = vec![0.0; self.config.motor_count as usize];
        }
    }

    /// Change flight mode
    pub fn set_flight_mode(&mut self, mode: FpvFlightMode) {
        self.flight_mode = mode;
    }

    /// Extract current roll, pitch, yaw angles from the orientation quaternion (in radians)
    fn euler_angles(&self) -> (f64, f64, f64) {
        let q = self.orientation;
        let (roll, pitch, yaw) = q.euler_angles();
        (roll, pitch, yaw)
    }

    /// Step the physics simulation
    ///
    /// `dt` is the time step in seconds (e.g., 0.001 for 1kHz).
    /// `input` is the current stick state.
    pub fn step(&mut self, dt: f64, input: &FpvStickInput) {
        if !self.armed {
            return;
        }

        let mass = self.config.mass_kg();
        let gravity = Vec3::new(0.0, 0.0, -9.81);

        // ── Compute target angular rates based on flight mode ──
        // Note: pitch input is negated to match convention (-1 = forward/nose down)
        // In the physics, positive pitch angle = nose down (rotation around Y with Z-up)
        let (target_roll_rate, target_pitch_rate, target_yaw_rate) = match self.flight_mode {
            FpvFlightMode::Acro => {
                // Direct rate control — stick maps to angular rate
                let roll = self.config.rates.roll_rate(input.roll).to_radians();
                let pitch = self.config.rates.pitch_rate(-input.pitch).to_radians();
                let yaw = self.config.rates.yaw_rate(input.yaw).to_radians();
                (roll, pitch, yaw)
            }
            FpvFlightMode::Angle => {
                // Self-leveling — stick maps to target angle, PID drives rates
                let max_angle = self.config.max_angle_deg.to_radians();
                let target_roll = input.roll * max_angle;
                let target_pitch = -input.pitch * max_angle; // Negate: -stick = nose down = +angle

                let (cur_roll, cur_pitch, _cur_yaw) = self.euler_angles();
                let roll_angle_error = target_roll - cur_roll;
                let pitch_angle_error = target_pitch - cur_pitch;

                // P controller on angle error → target rate (5.0 rad/s per rad error)
                let angle_p = 5.0;
                let roll_rate = (roll_angle_error * angle_p).clamp(
                    -self.config.rates.roll_super_rate.to_radians(),
                    self.config.rates.roll_super_rate.to_radians(),
                );
                let pitch_rate = (pitch_angle_error * angle_p).clamp(
                    -self.config.rates.pitch_super_rate.to_radians(),
                    self.config.rates.pitch_super_rate.to_radians(),
                );
                let yaw = self.config.rates.yaw_rate(input.yaw).to_radians();
                (roll_rate, pitch_rate, yaw)
            }
            FpvFlightMode::Horizon => {
                // Hybrid: self-level near center, acro at extremes
                let max_angle = self.config.max_angle_deg.to_radians();
                let (cur_roll, cur_pitch, _cur_yaw) = self.euler_angles();

                // Blending factor: 0 at center (angle mode), 1 at full deflection (acro)
                let roll_blend = input.roll.abs();
                let pitch_blend = input.pitch.abs();

                // Angle mode target rates (pitch negated for correct convention)
                let target_roll_a = input.roll * max_angle;
                let target_pitch_a = -input.pitch * max_angle;
                let angle_p = 5.0;
                let roll_rate_angle = ((target_roll_a - cur_roll) * angle_p).clamp(
                    -self.config.rates.roll_super_rate.to_radians(),
                    self.config.rates.roll_super_rate.to_radians(),
                );
                let pitch_rate_angle = ((target_pitch_a - cur_pitch) * angle_p).clamp(
                    -self.config.rates.pitch_super_rate.to_radians(),
                    self.config.rates.pitch_super_rate.to_radians(),
                );

                // Acro mode target rates (pitch negated)
                let roll_rate_acro = self.config.rates.roll_rate(input.roll).to_radians();
                let pitch_rate_acro = self.config.rates.pitch_rate(-input.pitch).to_radians();

                // Blend
                let roll_rate = roll_rate_angle * (1.0 - roll_blend) + roll_rate_acro * roll_blend;
                let pitch_rate =
                    pitch_rate_angle * (1.0 - pitch_blend) + pitch_rate_acro * pitch_blend;
                let yaw = self.config.rates.yaw_rate(input.yaw).to_radians();
                (roll_rate, pitch_rate, yaw)
            }
        };

        // ── Simplified PID on angular rates ──
        let rate_error = Vec3::new(
            target_roll_rate - self.angular_velocity.x,
            target_pitch_rate - self.angular_velocity.y,
            target_yaw_rate - self.angular_velocity.z,
        );

        let p_gains = Vec3::new(
            self.config.pid_roll.p / 100.0,
            self.config.pid_pitch.p / 100.0,
            self.config.pid_yaw.p / 100.0,
        );

        let torque = Vec3::new(
            rate_error.x * p_gains.x * self.config.inertia.x,
            rate_error.y * p_gains.y * self.config.inertia.y,
            rate_error.z * p_gains.z * self.config.inertia.z,
        );

        // ── Angular velocity update ──
        let angular_accel = Vec3::new(
            torque.x / self.config.inertia.x,
            torque.y / self.config.inertia.y,
            torque.z / self.config.inertia.z,
        );
        self.angular_velocity += angular_accel * dt;

        // Angular damping
        self.angular_velocity *= (1.0 - 0.1 * dt).max(0.0);

        // ── Orientation update (integrate angular velocity in body frame) ──
        let omega = self.angular_velocity;
        let omega_mag = omega.norm();
        if omega_mag > 1e-8 {
            let angle = omega_mag * dt;
            let axis = nalgebra::Unit::new_normalize(omega);
            let delta_rot = Rotation::from_axis_angle(&axis, angle);
            // Body-frame rotation: angular velocity is in body frame,
            // so apply delta on the right (intrinsic rotation)
            self.orientation = self.orientation * delta_rot;
        }

        // ── Thrust ──
        let throttle = self.config.rates.throttle_curve(input.throttle);
        let thrust_magnitude = throttle * self.config.max_thrust_n();

        // Thrust is along the drone's local Z axis (up)
        let thrust_direction = self.orientation * Vec3::new(0.0, 0.0, 1.0);
        let thrust_force = thrust_direction * thrust_magnitude;

        // ── Drag ──
        let speed = self.velocity.norm();
        let drag_force = if speed > 0.01 {
            -self.velocity.normalize() * self.config.drag_coefficient * speed * speed
        } else {
            Vec3::zeros()
        };

        // ── Linear acceleration ──
        let total_force = thrust_force + gravity * mass + drag_force;
        let acceleration = total_force / mass;

        // ── Update velocity and position ──
        self.velocity += acceleration * dt;
        self.position += self.velocity * dt;

        // ── Ground collision ──
        if self.position.z < 0.0 {
            self.position.z = 0.0;
            self.velocity.z = 0.0_f64.max(self.velocity.z);
            // Friction on ground
            self.velocity.x *= (1.0 - 5.0 * dt).max(0.0);
            self.velocity.y *= (1.0 - 5.0 * dt).max(0.0);
        }

        // ── Motor outputs (simplified — uniform for now) ──
        for m in &mut self.motor_outputs {
            *m = throttle;
        }

        // ── Battery simulation ──
        let current_draw = throttle * 30.0 + 2.0; // Amps (simplified)
        self.mah_consumed += current_draw * 1000.0 * dt / 3600.0;
        self.battery_remaining =
            (1.0 - self.mah_consumed / self.config.battery_capacity_mah as f64).max(0.0);
        // Voltage sag under load
        let nominal = self.config.battery_cells as f64 * 3.7;
        let full = self.config.battery_cells as f64 * 4.2;
        self.battery_voltage =
            full * self.battery_remaining - current_draw * 0.01 * self.config.battery_cells as f64;
        self.battery_voltage = self.battery_voltage.max(nominal * 0.8);

        self.flight_time += dt;
    }

    /// Get current FPV state snapshot
    pub fn state(&self, vehicle_id: &str) -> FpvState {
        let speed = self.velocity.norm();
        let alt = self.position.z - self.start_altitude;

        let mut warnings = Vec::new();
        if self.battery_remaining < 0.2 {
            warnings.push("LOW BATTERY".to_string());
        }
        if self.battery_voltage < self.config.battery_cells as f64 * 3.5 {
            warnings.push("VOLTAGE WARN".to_string());
        }

        let mode_str = match self.flight_mode {
            FpvFlightMode::Acro => "ACRO",
            FpvFlightMode::Angle => "ANGL",
            FpvFlightMode::Horizon => "HOR",
        };

        FpvState {
            vehicle_id: vehicle_id.to_string(),
            position: self.position,
            orientation: self.orientation,
            velocity: self.velocity,
            angular_velocity: self.angular_velocity,
            motor_rpms: self.motor_outputs.iter().map(|o| o * 25000.0).collect(),
            motor_outputs: self.motor_outputs.clone(),
            battery_remaining: self.battery_remaining,
            battery_voltage: self.battery_voltage,
            flight_mode: self.flight_mode,
            armed: self.armed,
            osd: FpvOsd {
                battery_voltage: self.battery_voltage,
                current_amps: self.motor_outputs.iter().sum::<f64>() * 30.0 + 2.0,
                mah_consumed: self.mah_consumed,
                rssi: 99,
                flight_mode: mode_str.to_string(),
                altitude_m: alt,
                ground_speed_mps: (self.velocity.x * self.velocity.x
                    + self.velocity.y * self.velocity.y)
                    .sqrt(),
                flight_time_s: self.flight_time,
                gps_sats: 14,
                throttle_pct: (self.motor_outputs.first().copied().unwrap_or(0.0) * 100.0) as u8,
                craft_name: self.config.name.clone(),
                warnings,
                show_crosshair: true,
            },
            timestamp: self.flight_time,
            speed_mps: speed,
            altitude_m: alt,
        }
    }

    /// Get the FPV camera transform in world space
    /// (accounts for camera tilt and drone orientation)
    pub fn camera_transform(&self) -> Transform {
        let cam = &self.config.camera;

        // Camera is mounted tilted up by `tilt_angle_deg` around the local Y axis
        let tilt = Rotation::from_euler_angles(0.0, cam.tilt_angle_deg.to_radians(), 0.0);
        let cam_orientation = self.orientation * tilt;

        // Camera position = drone position + rotated offset
        let cam_offset_world = self.orientation * cam.offset;
        let cam_position = self.position + cam_offset_world;

        Transform::new(cam_position, cam_orientation)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use nalgebra::Point3;

    #[test]
    fn test_rates_expo() {
        // Zero expo = linear
        assert!((RatesProfile::apply_expo(0.5, 0.0) - 0.5).abs() < 1e-10);
        assert!((RatesProfile::apply_expo(1.0, 0.0) - 1.0).abs() < 1e-10);
        assert!((RatesProfile::apply_expo(-1.0, 0.0) - (-1.0)).abs() < 1e-10);

        // With expo, center should be reduced, full deflection unchanged
        let expo_val = RatesProfile::apply_expo(0.5, 0.5);
        assert!(expo_val.abs() < 0.5); // Should be less than linear
        assert!((RatesProfile::apply_expo(1.0, 0.5) - 1.0).abs() < 1e-10);
    }

    #[test]
    fn test_race_rates_profile() {
        let rates = RatesProfile::race();
        // At full deflection, rate should be very high
        let max_roll = rates.roll_rate(1.0);
        assert!(max_roll > 500.0); // > 500 deg/s
                                   // At center, rate should be 0
        assert!((rates.roll_rate(0.0)).abs() < 1e-10);
    }

    #[test]
    fn test_fpv_drone_configs() {
        let race = FpvDroneConfig::five_inch_race();
        assert!(race.thrust_to_weight() > 5.0); // 5:1+ for racing
        assert_eq!(race.motor_count, 4);
        assert_eq!(race.battery_cells, 6);

        let micro = FpvDroneConfig::three_inch_micro();
        assert!(micro.weight_grams < 300.0);
        assert!(micro.thrust_to_weight() > 5.0);
    }

    #[test]
    fn test_fpv_physics_hover() {
        let config = FpvDroneConfig::five_inch_race();
        let spawn = Point3::new(0.0, 0.0, 10.0);
        let mut physics = FpvPhysics::new(config, spawn);

        physics.set_armed(true);

        // Hover for 1 second
        let input = FpvStickInput::hover();
        for _ in 0..1000 {
            physics.step(0.001, &input);
        }

        // Drone should still be roughly near starting height
        // (may drift slightly due to simplified physics)
        assert!(
            physics.position.z > 5.0,
            "Drone fell too far: z={}",
            physics.position.z
        );
        assert!(
            physics.position.z < 20.0,
            "Drone flew too high: z={}",
            physics.position.z
        );
    }

    #[test]
    fn test_fpv_physics_no_arm() {
        let config = FpvDroneConfig::five_inch_race();
        let spawn = Point3::new(0.0, 0.0, 10.0);
        let mut physics = FpvPhysics::new(config, spawn);

        // Not armed — stick input should have no effect
        let input = FpvStickInput::new(1.0, 1.0, 1.0, 1.0);
        for _ in 0..100 {
            physics.step(0.001, &input);
        }

        // Position should not change (gravity may push down but motors off)
        assert_eq!(physics.motor_outputs, vec![0.0; 4]);
    }

    #[test]
    fn test_camera_transform() {
        let config = FpvDroneConfig::five_inch_race();
        let spawn = Point3::new(0.0, 0.0, 10.0);
        let physics = FpvPhysics::new(config, spawn);

        let cam = physics.camera_transform();
        // Camera should be near spawn with some tilt
        assert!((cam.position.z - 10.0).abs() < 0.1);
    }

    #[test]
    fn test_fpv_osd() {
        let osd = FpvOsd::default();
        assert!(osd.battery_voltage > 24.0);
        assert_eq!(osd.flight_mode, "ACRO");
        assert!(osd.warnings.is_empty());
    }

    #[test]
    fn test_stick_input_clamp() {
        let input = FpvStickInput::new(2.0, -3.0, 5.0, -10.0);
        assert_eq!(input.throttle, 1.0);
        assert_eq!(input.roll, -1.0);
        assert_eq!(input.pitch, 1.0);
        assert_eq!(input.yaw, -1.0);
    }
}
