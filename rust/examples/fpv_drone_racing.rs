//! FPV Drone Racing Simulation — AutonomySim
//!
//! A first-person-view drone racing demo similar to TRYP FPV and Liftoff.
//! Features:
//! - Acro / Angle / Horizon flight modes
//! - Betaflight-style rate profiles with expo curves
//! - Realistic FPV camera with configurable tilt angle and FOV
//! - OSD (On Screen Display) telemetry overlay
//! - Battery simulation with voltage sag
//! - Multiple drone presets (5" race, 5" freestyle, 3" micro, 7" LR)
//!
//! # Running
//!
//! ## Headless (no UE5 needed)
//! ```sh
//! cargo run --example fpv_drone_racing --release
//! ```
//!
//! ## With Unreal Engine 5 visualization
//! ```sh
//! cargo run --example fpv_drone_racing --features unreal --release
//! ```

use autonomysim_core::fpv::*;
use autonomysim_core::prelude::*;
use nalgebra::Point3;
use std::time::Instant;
use tracing::info;

#[cfg(feature = "unreal")]
use autonomysim_backends::unreal::UnrealEngine5Backend;

/// Race track gate definition
#[derive(Debug, Clone)]
struct RaceGate {
    position: Point3<f64>,
    _yaw_deg: f64,
    width: f64,
    _height: f64,
    _index: usize,
}

/// FPV racing simulation
struct FpvRacingSim {
    /// Drone physics
    physics: FpvPhysics,

    /// Race track gates
    gates: Vec<RaceGate>,

    /// Next gate to pass through
    next_gate: usize,

    /// Gates passed
    gates_passed: usize,

    /// Lap count
    laps: usize,

    /// Total laps in race
    total_laps: usize,

    /// Lap times in seconds
    lap_times: Vec<f64>,

    /// Current lap start time
    lap_start_time: f64,

    /// Best lap time
    best_lap: Option<f64>,

    /// Race started
    race_started: bool,

    /// Unreal Engine backend
    #[cfg(feature = "unreal")]
    unreal: Option<UnrealEngine5Backend>,
}

impl FpvRacingSim {
    /// Create a new FPV racing simulation
    fn new(drone_config: FpvDroneConfig, total_laps: usize) -> Self {
        let spawn = Point3::new(0.0, 0.0, 1.5);
        let physics = FpvPhysics::new(drone_config, spawn);
        let gates = Self::create_race_track();

        Self {
            physics,
            gates,
            next_gate: 0,
            gates_passed: 0,
            laps: 0,
            total_laps,
            lap_times: Vec::new(),
            lap_start_time: 0.0,
            best_lap: None,
            race_started: false,
            #[cfg(feature = "unreal")]
            unreal: None,
        }
    }

    /// Create a simple oval race track with gates
    fn create_race_track() -> Vec<RaceGate> {
        let mut gates = Vec::new();

        // Simple oval track with 8 gates
        let track_points: Vec<(f64, f64, f64, f64)> = vec![
            (0.0, 20.0, 3.0, 0.0),    // Gate 0: straight
            (15.0, 35.0, 4.0, 45.0),  // Gate 1: entering turn
            (30.0, 40.0, 5.0, 90.0),  // Gate 2: apex
            (45.0, 35.0, 3.0, 135.0), // Gate 3: exit turn
            (50.0, 20.0, 4.0, 180.0), // Gate 4: back straight
            (45.0, 5.0, 3.0, 225.0),  // Gate 5: entering turn 2
            (30.0, 0.0, 5.0, 270.0),  // Gate 6: low gate (dive)
            (15.0, 5.0, 4.0, 315.0),  // Gate 7: return
        ];

        for (i, (x, y, z, yaw)) in track_points.iter().enumerate() {
            gates.push(RaceGate {
                position: Point3::new(*x, *y, *z),
                _yaw_deg: *yaw,
                width: 3.0,
                _height: 3.0,
                _index: i,
            });
        }

        gates
    }

    /// Check if the drone has passed through the next gate
    fn check_gate_passage(&mut self) {
        if self.gates.is_empty() || !self.race_started {
            return;
        }

        let gate = &self.gates[self.next_gate];
        let pos = self.physics.position;
        let dx = pos.x - gate.position.x;
        let dy = pos.y - gate.position.y;
        let dz = pos.z - gate.position.z;
        let dist = (dx * dx + dy * dy + dz * dz).sqrt();

        // Proximity check — pass within 1.5x gate radius
        if dist < gate.width * 1.5 {
            self.gates_passed += 1;
            info!(
                "🏁 Gate {} passed! (total: {}, distance: {:.1}m)",
                self.next_gate, self.gates_passed, dist
            );

            self.next_gate += 1;
            if self.next_gate >= self.gates.len() {
                self.next_gate = 0;
                self.laps += 1;
                let lap_time = self.physics.flight_time - self.lap_start_time;
                self.lap_times.push(lap_time);
                self.lap_start_time = self.physics.flight_time;

                let is_best = match self.best_lap {
                    Some(best) => lap_time < best,
                    None => true,
                };
                if is_best {
                    self.best_lap = Some(lap_time);
                }

                info!(
                    "🏆 Lap {} complete! Time: {:.2}s {}",
                    self.laps,
                    lap_time,
                    if is_best { "(BEST!)" } else { "" }
                );
            }
        }
    }

    /// Generate an autopilot input that flies toward the next gate.
    /// Uses Angle mode for stable self-leveling flight.
    fn autopilot_input(&self) -> FpvStickInput {
        if self.gates.is_empty() {
            return FpvStickInput::hover();
        }

        let gate = &self.gates[self.next_gate];
        let pos = self.physics.position;

        // Direction to gate (horizontal)
        let dx = gate.position.x - pos.x;
        let dy = gate.position.y - pos.y;
        let dist_h = (dx * dx + dy * dy).sqrt();

        // ── Compute heading to target ──
        // atan2(y, x) gives angle from +X axis, counter-clockwise positive
        let target_yaw = dy.atan2(dx);

        // Current heading from drone orientation
        let fwd = self.physics.orientation * Vec3::new(1.0, 0.0, 0.0);
        let current_yaw = fwd.y.atan2(fwd.x);

        // Yaw error — wrapped to [-pi, pi]
        let mut yaw_err = target_yaw - current_yaw;
        while yaw_err > std::f64::consts::PI {
            yaw_err -= 2.0 * std::f64::consts::PI;
        }
        while yaw_err < -std::f64::consts::PI {
            yaw_err += 2.0 * std::f64::consts::PI;
        }

        // ── Angle mode control ──
        // Yaw: aggressive correction to face the gate
        // Positive yaw_err → need to yaw left (positive rotation around Z)
        // Positive yaw stick → positive yaw rate → positive Z angular velocity
        let yaw_stick = (yaw_err * 2.0).clamp(-1.0, 1.0);

        // Only pitch forward when roughly aligned with target.
        let alignment = 1.0 - (yaw_err.abs() / std::f64::consts::FRAC_PI_2).min(1.0);

        // Forward pitch scales with alignment and distance
        let max_pitch = if dist_h > 15.0 {
            -0.5
        } else if dist_h > 5.0 {
            -0.30
        } else {
            -0.10
        };
        let pitch_stick = max_pitch * alignment;

        // Coordinated turn: bank into the turn proportional to yaw error
        let roll_stick = (yaw_err * 0.2 * alignment).clamp(-0.3, 0.3);

        // Altitude control using correct hover throttle
        let hover_stick = self.physics.config.hover_throttle_input();
        let target_alt = gate.position.z;
        let alt_err = target_alt - pos.z;
        let vertical_speed = self.physics.velocity.z;
        // PD altitude controller with small corrections around hover
        let throttle = (hover_stick + alt_err * 0.03 - vertical_speed * 0.01).clamp(0.0, 0.50);

        FpvStickInput::new(throttle, roll_stick, pitch_stick, yaw_stick)
    }

    /// Run the simulation
    async fn run(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        let vehicle_id = "fpv_racer_1";
        let dt = 0.001; // 1kHz physics
        let sim_duration_s = 120.0; // 2 minutes
        let total_steps = (sim_duration_s / dt) as usize;
        let print_interval = 1000; // Print every 1s of sim time
        let _ue5_update_interval = 16; // ~60Hz updates to UE5

        info!("═══════════════════════════════════════════════════════════");
        info!("     AutonomySim FPV Drone Racing Simulation");
        info!("═══════════════════════════════════════════════════════════");
        info!("");
        info!("  Drone:   {}", self.physics.config.name);
        info!("  T/W:     {:.1}:1", self.physics.config.thrust_to_weight());
        info!(
            "  Battery: {}S {}mAh",
            self.physics.config.battery_cells, self.physics.config.battery_capacity_mah
        );
        info!("  Mode:    {:?}", self.physics.flight_mode);
        info!(
            "  Rates:   {} (Roll: {:.0}°/s max, Pitch: {:.0}°/s max)",
            self.physics.config.rates.name,
            self.physics.config.rates.roll_rate(1.0),
            self.physics.config.rates.pitch_rate(1.0)
        );
        info!(
            "  Camera:  {:.0}° tilt, {:.0}° FOV",
            self.physics.config.camera.tilt_angle_deg, self.physics.config.camera.fov_h_deg
        );
        info!(
            "  Track:   {} gates, {} laps",
            self.gates.len(),
            self.total_laps
        );
        info!(
            "  Hover:   {:.1}% stick ({:.1}% thrust)",
            self.physics.config.hover_throttle_input() * 100.0,
            self.physics.config.mass_kg() * 9.81 / self.physics.config.max_thrust_n() * 100.0
        );
        info!("");

        // Connect to UE5 if feature enabled
        #[cfg(feature = "unreal")]
        {
            info!("Connecting to Unreal Engine 5...");
            match autonomysim_backends::unreal::UnrealConnection::connect("127.0.0.1", 41451).await
            {
                Ok(conn) => {
                    info!("✓ Connected to UE5!");

                    // Spawn FPV drone in UE5
                    let _ = conn
                        .spawn_fpv_drone(
                            vehicle_id,
                            "5inch_race",
                            self.physics.position.x,
                            self.physics.position.y,
                            self.physics.position.z,
                            0.0,
                        )
                        .await;

                    // Configure FPV camera
                    let cam = &self.physics.config.camera;
                    let _ = conn
                        .set_fpv_camera(
                            vehicle_id,
                            cam.tilt_angle_deg,
                            cam.fov_h_deg,
                            cam.resolution_width,
                            cam.resolution_height,
                            cam.lens_distortion,
                            cam.latency_ms,
                        )
                        .await;

                    info!("✓ FPV drone spawned in UE5");
                }
                Err(e) => {
                    tracing::warn!("Could not connect to UE5 (running headless): {}", e);
                }
            }
        }

        // Arm and start
        info!("Arming drone...");
        self.physics.set_armed(true);
        // Use Angle mode for autopilot (self-leveling for stable navigation)
        self.physics.set_flight_mode(FpvFlightMode::Angle);
        self.race_started = true;
        self.lap_start_time = 0.0;

        info!("Race started! Flying {} laps...", self.total_laps);
        info!("");

        let real_start = Instant::now();
        let mut step = 0u64;

        while step < total_steps as u64 && self.laps < self.total_laps {
            // Autopilot generates stick input
            let input = self.autopilot_input();

            // Step physics
            self.physics.step(dt, &input);

            // Check gate passage
            self.check_gate_passage();

            // Periodic logging
            if step % print_interval as u64 == 0 && step > 0 {
                let state = self.physics.state(vehicle_id);
                info!(
                    "  t={:.1}s | pos=({:.1}, {:.1}, {:.1}) | spd={:.1}m/s | alt={:.1}m | bat={:.0}% ({:.1}V) | mode={} | gate={}/{}  |  lap={}/{}",
                    state.timestamp,
                    state.position.x,
                    state.position.y,
                    state.position.z,
                    state.speed_mps,
                    state.altitude_m,
                    state.battery_remaining * 100.0,
                    state.battery_voltage,
                    state.osd.flight_mode,
                    self.next_gate,
                    self.gates.len(),
                    self.laps,
                    self.total_laps,
                );
            }

            step += 1;
        }

        let real_elapsed = real_start.elapsed().as_secs_f64();
        let sim_elapsed = self.physics.flight_time;

        info!("");
        info!("═══════════════════════════════════════════════════════════");
        info!("                    RACE RESULTS");
        info!("═══════════════════════════════════════════════════════════");
        info!("  Laps completed:   {}/{}", self.laps, self.total_laps);
        info!("  Gates passed:     {}", self.gates_passed);
        if let Some(best) = self.best_lap {
            info!("  Best lap:         {:.2}s", best);
        }
        for (i, lt) in self.lap_times.iter().enumerate() {
            info!("  Lap {}:            {:.2}s", i + 1, lt);
        }
        info!("");
        info!("  Sim time:         {:.1}s", sim_elapsed);
        info!("  Real time:        {:.3}s", real_elapsed);
        info!(
            "  Real-time ratio:  {:.0}x",
            sim_elapsed / real_elapsed.max(0.001)
        );
        info!(
            "  Physics rate:     {:.0} steps/s",
            step as f64 / real_elapsed.max(0.001)
        );
        info!(
            "  Battery used:     {:.0}mAh ({:.0}% remaining)",
            self.physics.mah_consumed,
            self.physics.battery_remaining * 100.0
        );
        info!("  Final voltage:    {:.1}V", self.physics.battery_voltage);
        info!("═══════════════════════════════════════════════════════════");

        Ok(())
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize logging
    tracing_subscriber::fmt()
        .with_max_level(tracing::Level::INFO)
        .init();

    info!("AutonomySim FPV Drone Racing Demo");
    info!("Inspired by TRYP FPV and Liftoff");
    info!("");

    // ── Demo 1: 5" Race Quad ─────────────────────────────────────────────
    info!("━━━ Demo 1: 5\" Race Quad ━━━");
    let config = FpvDroneConfig::five_inch_race();
    let mut sim = FpvRacingSim::new(config, 3);
    sim.run().await?;

    info!("");

    // ── Demo 2: 3" Micro (different handling) ────────────────────────────
    info!("━━━ Demo 2: 3\" Micro Quad ━━━");
    let config = FpvDroneConfig::three_inch_micro();
    let mut sim = FpvRacingSim::new(config, 2);
    sim.run().await?;

    info!("");

    // ── Demo 3: Rate profiles comparison ─────────────────────────────────
    info!("━━━ Rate Profile Comparison ━━━");
    info!("");
    for profile in [
        RatesProfile::race(),
        RatesProfile::freestyle(),
        RatesProfile::cinematic(),
    ] {
        info!("  {} rates:", profile.name);
        info!(
            "    Roll:  center={:.0}°/s  max={:.0}°/s  expo={:.1}",
            profile.roll_rc_rate,
            profile.roll_rate(1.0),
            profile.roll_expo
        );
        info!(
            "    Pitch: center={:.0}°/s  max={:.0}°/s  expo={:.1}",
            profile.pitch_rc_rate,
            profile.pitch_rate(1.0),
            profile.pitch_expo
        );
        info!(
            "    Yaw:   center={:.0}°/s  max={:.0}°/s  expo={:.1}",
            profile.yaw_rc_rate,
            profile.yaw_rate(1.0),
            profile.yaw_expo
        );
        info!("");
    }

    // ── Demo 4: Drone presets comparison ─────────────────────────────────
    info!("━━━ Drone Preset Comparison ━━━");
    info!("");
    for drone in [
        FpvDroneConfig::three_inch_micro(),
        FpvDroneConfig::five_inch_race(),
        FpvDroneConfig::five_inch_freestyle(),
        FpvDroneConfig::seven_inch_longrange(),
    ] {
        info!("  {}:", drone.name);
        info!(
            "    Frame: {:.0}mm | Weight: {:.0}g | T/W: {:.1}:1",
            drone.frame_size_mm,
            drone.weight_grams,
            drone.thrust_to_weight()
        );
        info!(
            "    Motors: {}x {}KV | Props: {:.0}\" | Battery: {}S {}mAh",
            drone.motor_count,
            drone.motor_kv,
            drone.prop_size_inches,
            drone.battery_cells,
            drone.battery_capacity_mah
        );
        info!(
            "    Camera: {:.0}° tilt | {:.0}° FOV | {}x{}",
            drone.camera.tilt_angle_deg,
            drone.camera.fov_h_deg,
            drone.camera.resolution_width,
            drone.camera.resolution_height
        );
        info!("");
    }

    Ok(())
}
