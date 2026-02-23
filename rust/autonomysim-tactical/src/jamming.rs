//! Jamming Models for Electronic Warfare
//!
//! Implements three primary jamming techniques:
//! - **Barrage Jamming**: Continuous wideband noise over target bandwidth
//! - **Follower Jamming**: Track and jam specific frequency
//! - **Swept Jamming**: Periodic frequency sweep
//!
//! Based on military EW doctrine and communication theory.

use nalgebra::Vector3;
use serde::{Deserialize, Serialize};
use std::f64::consts::PI;

use autonomysim_rf_core::utils::dbm_to_watts;

/// Jamming technique type
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum JammingType {
    /// Barrage: Wideband noise over entire target bandwidth
    /// Most effective against frequency-hopping systems
    Barrage,

    /// Follower: Track and jam specific frequency
    /// Effective against fixed-frequency systems
    Follower {
        /// Tracking time constant (seconds)
        tracking_time: f64,
    },

    /// Swept: Periodic frequency sweep
    /// Effective when jammer power limited
    Swept {
        /// Sweep rate (Hz/s)
        sweep_rate: f64,
        /// Sweep period (seconds)
        sweep_period: f64,
    },

    /// Pulse: High-power pulses
    /// Effective against radar/communication systems
    Pulse {
        /// Pulse repetition frequency (Hz)
        prf: f64,
        /// Pulse width (seconds)
        pulse_width: f64,
        /// Peak power (dBm)
        peak_power_dbm: f64,
    },

    /// Deception: Mimics legitimate signals
    /// Most sophisticated, hardest to detect
    Deception {
        /// Delay before retransmission (seconds)
        delay: f64,
    },
}

/// Jamming configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct JammingConfig {
    /// Jammer position (meters, NED frame)
    pub jammer_position: Vector3<f64>,

    /// Jammer transmit power (dBm)
    pub jammer_power_dbm: f64,

    /// Type of jamming
    pub jamming_type: JammingType,

    /// Center frequency (Hz)
    pub center_frequency_hz: f64,

    /// Jamming bandwidth (Hz)
    pub bandwidth_hz: f64,

    /// Antenna gain (dBi)
    pub antenna_gain_dbi: f64,

    /// Polarization mismatch loss (dB)
    pub polarization_loss_db: f64,

    /// Atmospheric attenuation (dB/km)
    pub atmospheric_loss_db_per_km: f64,

    /// Enable terrain masking
    pub enable_terrain_masking: bool,
}

impl Default for JammingConfig {
    fn default() -> Self {
        Self {
            jammer_position: Vector3::zeros(),
            jammer_power_dbm: 40.0, // 10W
            jamming_type: JammingType::Barrage,
            center_frequency_hz: 2.4e9, // 2.4 GHz
            bandwidth_hz: 20e6,         // 20 MHz
            antenna_gain_dbi: 10.0,     // Directional antenna
            polarization_loss_db: 3.0,
            atmospheric_loss_db_per_km: 0.1,
            enable_terrain_masking: true,
        }
    }
}

/// Jamming-to-Signal Ratio (J/S) thresholds
pub mod jsr_thresholds {
    /// J/S > 10 dB: Communication completely denied
    pub const COMPLETE_DENIAL: f64 = 10.0;

    /// J/S > 6 dB: Severe degradation, <10% throughput
    pub const SEVERE_DEGRADATION: f64 = 6.0;

    /// J/S > 3 dB: Moderate degradation, ~50% throughput
    pub const MODERATE_DEGRADATION: f64 = 3.0;

    /// J/S > 0 dB: Marginal operation, increased errors
    pub const MARGINAL: f64 = 0.0;
}

/// Jamming model for electronic warfare
pub struct JammingModel {
    config: JammingConfig,
    /// Current time for time-varying jamming
    current_time: f64,
}

impl JammingModel {
    /// Create a new jamming model
    pub fn new(config: JammingConfig) -> Self {
        Self {
            config,
            current_time: 0.0,
        }
    }

    /// Update simulation time (for swept/pulse jamming)
    pub fn update_time(&mut self, time: f64) {
        self.current_time = time;
    }

    /// Compute jamming power at target position (dBm)
    ///
    /// Uses Friis transmission equation with additional losses:
    /// Pr = Pt + Gt + Gr - 20*log10(4πd/λ) - L_atm - L_pol
    pub fn compute_jamming_power(&self, target_position: Vector3<f64>) -> f64 {
        // Distance from jammer to target
        let distance_m = (target_position - self.config.jammer_position).norm();

        if distance_m < 1.0 {
            // Too close, return maximum power
            return self.config.jammer_power_dbm + self.config.antenna_gain_dbi;
        }

        // Wavelength (m)
        let wavelength = 3e8 / self.config.center_frequency_hz;

        // Free space path loss (dB)
        let fspl_db = 20.0 * ((4.0 * PI * distance_m) / wavelength).log10();

        // Atmospheric attenuation (dB)
        let distance_km = distance_m / 1000.0;
        let atmospheric_loss_db = self.config.atmospheric_loss_db_per_km * distance_km;

        // Received jamming power
        let mut jamming_power_dbm = self.config.jammer_power_dbm + self.config.antenna_gain_dbi
            - fspl_db
            - atmospheric_loss_db
            - self.config.polarization_loss_db;

        // Apply jamming type specific effects
        jamming_power_dbm += self.compute_type_specific_gain();

        // Terrain masking (simplified - assumes ground plane)
        if self.config.enable_terrain_masking {
            let jammer_height = self.config.jammer_position.z.abs();
            let target_height = target_position.z.abs();

            if jammer_height < 10.0 && target_height < 10.0 {
                // Both near ground, additional loss
                jamming_power_dbm -= 10.0;
            }
        }

        jamming_power_dbm
    }

    /// Compute type-specific power adjustment
    fn compute_type_specific_gain(&self) -> f64 {
        match self.config.jamming_type {
            JammingType::Barrage => {
                // Barrage spreads power over bandwidth
                // Effective power is reduced by processing gain
                let processing_gain_db = 10.0 * (self.config.bandwidth_hz / 1e6).log10();
                -processing_gain_db.min(20.0) // Cap at 20 dB loss
            }

            JammingType::Follower { .. } => {
                // Follower concentrates power on single frequency
                // More effective than barrage
                3.0
            }

            JammingType::Swept { sweep_period, .. } => {
                // Swept jammer only effective when on target frequency
                // Duty cycle depends on sweep rate and target bandwidth
                let duty_cycle = (self.config.bandwidth_hz * sweep_period)
                    / (self.config.center_frequency_hz * 0.1); // Assume 10% frequency range
                let duty_cycle_clamped = duty_cycle.clamp(0.01, 1.0);

                // Convert duty cycle to dB
                10.0 * duty_cycle_clamped.log10()
            }

            JammingType::Pulse {
                prf,
                pulse_width,
                peak_power_dbm,
            } => {
                // Pulse jammer: high peak power, low duty cycle
                let duty_cycle = prf * pulse_width;
                let duty_cycle_clamped = duty_cycle.clamp(0.001, 1.0);

                // Average power = peak power * duty cycle
                let avg_power_adjustment = 10.0 * duty_cycle_clamped.log10();

                // But peak power can be much higher
                let peak_power_delta = peak_power_dbm - self.config.jammer_power_dbm;

                // Return average effect (conservative)
                avg_power_adjustment + peak_power_delta * duty_cycle_clamped
            }

            JammingType::Deception { .. } => {
                // Deception jamming is most effective
                // Appears as legitimate signal
                10.0
            }
        }
    }

    /// Compute Jamming-to-Signal Ratio (J/S) in dB
    ///
    /// J/S = 10*log10(Pj/Ps)
    ///
    /// where:
    /// - Pj = Jamming power (W)
    /// - Ps = Signal power (W)
    ///
    /// J/S > 10 dB indicates effective jamming (communication denial)
    pub fn compute_jamming_to_signal_ratio(
        &self,
        signal_power_dbm: f64,
        jamming_power_dbm: f64,
    ) -> f64 {
        jamming_power_dbm - signal_power_dbm
    }

    /// Determine if jamming is effective at target position
    ///
    /// Returns true if J/S > 10 dB (complete denial)
    pub fn is_effective(&self, signal_power_dbm: f64, target_position: Vector3<f64>) -> bool {
        let jamming_power_dbm = self.compute_jamming_power(target_position);
        let jsr_db = self.compute_jamming_to_signal_ratio(signal_power_dbm, jamming_power_dbm);
        jsr_db > jsr_thresholds::COMPLETE_DENIAL
    }

    /// Compute communication throughput reduction (0.0 = no throughput, 1.0 = full throughput)
    ///
    /// Based on Shannon capacity with jamming:
    /// C = B * log2(1 + SNR)
    ///
    /// With jamming, SNR becomes SINR (Signal-to-Interference-plus-Noise Ratio)
    pub fn compute_throughput_reduction(
        &self,
        signal_power_dbm: f64,
        target_position: Vector3<f64>,
        noise_power_dbm: f64,
    ) -> f64 {
        let jamming_power_dbm = self.compute_jamming_power(target_position);

        // Convert to linear scale (watts)
        let signal_power_w = dbm_to_watts(signal_power_dbm);
        let jamming_power_w = dbm_to_watts(jamming_power_dbm);
        let noise_power_w = dbm_to_watts(noise_power_dbm);

        // Compute SNR without jamming
        let snr_no_jam = signal_power_w / noise_power_w;
        let capacity_no_jam = (1.0 + snr_no_jam).log2();

        // Compute SINR with jamming
        let sinr_with_jam = signal_power_w / (jamming_power_w + noise_power_w);
        let capacity_with_jam = (1.0 + sinr_with_jam).log2();

        // Throughput reduction
        let throughput_ratio = capacity_with_jam / capacity_no_jam;
        throughput_ratio.clamp(0.0, 1.0)
    }

    /// Compute packet error rate (PER) under jamming
    ///
    /// Uses approximation: PER ≈ 1 - (1 - BER)^N
    /// where N is packet length in bits
    pub fn compute_packet_error_rate(
        &self,
        signal_power_dbm: f64,
        target_position: Vector3<f64>,
        noise_power_dbm: f64,
        packet_length_bits: usize,
    ) -> f64 {
        let jamming_power_dbm = self.compute_jamming_power(target_position);

        // Convert to linear scale
        let signal_power_w = dbm_to_watts(signal_power_dbm);
        let jamming_power_w = dbm_to_watts(jamming_power_dbm);
        let noise_power_w = dbm_to_watts(noise_power_dbm);

        // SINR
        let sinr = signal_power_w / (jamming_power_w + noise_power_w);

        // BER approximation for BPSK: BER ≈ 0.5 * erfc(sqrt(SINR))
        // Simplified approximation: BER ≈ 0.5 * exp(-SINR)
        let ber = if sinr > 20.0 {
            1e-10 // Very low BER for high SINR
        } else if sinr < 0.01 {
            0.5 // Near 50% errors for very low SINR
        } else {
            0.5 * (-sinr).exp()
        };

        // PER from BER
        let per = 1.0 - (1.0 - ber).powi(packet_length_bits as i32);
        per.clamp(0.0, 1.0)
    }

    /// Get current configuration
    pub fn config(&self) -> &JammingConfig {
        &self.config
    }

    /// Update configuration
    pub fn set_config(&mut self, config: JammingConfig) {
        self.config = config;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use approx::assert_abs_diff_eq;

    #[test]
    fn test_barrage_jamming() {
        let config = JammingConfig {
            jammer_position: Vector3::new(0.0, 0.0, 100.0),
            jammer_power_dbm: 50.0, // 100W
            jamming_type: JammingType::Barrage,
            center_frequency_hz: 2.4e9,
            bandwidth_hz: 20e6,
            ..Default::default()
        };

        let jammer = JammingModel::new(config);

        // Test at 1 km distance
        let target = Vector3::new(1000.0, 0.0, 50.0);
        let jamming_power = jammer.compute_jamming_power(target);

        // Should have significant path loss
        // Barrage jamming has additional processing gain loss (up to -20 dB)
        // So received power can be quite low at distance
        assert!(jamming_power < 50.0);
        assert!(jamming_power > -100.0); // Still within reasonable range
    }

    #[test]
    fn test_jsr_calculation() {
        let config = JammingConfig::default();
        let jammer = JammingModel::new(config);

        let signal_dbm = -70.0;
        let jamming_dbm = -60.0;
        let jsr = jammer.compute_jamming_to_signal_ratio(signal_dbm, jamming_dbm);

        assert_abs_diff_eq!(jsr, 10.0, epsilon = 0.01);
    }

    #[test]
    fn test_effective_jamming() {
        let config = JammingConfig {
            jammer_position: Vector3::zeros(),
            jammer_power_dbm: 40.0,
            jamming_type: JammingType::Follower { tracking_time: 0.1 },
            ..Default::default()
        };

        let jammer = JammingModel::new(config);

        // At close range with weak signal, should be effective
        let target = Vector3::new(100.0, 0.0, 10.0);
        let signal_dbm = -80.0;

        assert!(jammer.is_effective(signal_dbm, target));
    }

    #[test]
    fn test_throughput_reduction() {
        let config = JammingConfig {
            jammer_position: Vector3::zeros(),
            jammer_power_dbm: 30.0,
            ..Default::default()
        };

        let jammer = JammingModel::new(config);
        let target = Vector3::new(500.0, 0.0, 50.0);
        let signal_dbm = -70.0;
        let noise_dbm = -100.0;

        let throughput = jammer.compute_throughput_reduction(signal_dbm, target, noise_dbm);

        // Should have reduced throughput
        assert!(throughput < 1.0);
        assert!(throughput > 0.0);
    }

    #[test]
    fn test_packet_error_rate() {
        let config = JammingConfig {
            jammer_position: Vector3::zeros(),
            jammer_power_dbm: 40.0,
            ..Default::default()
        };

        let jammer = JammingModel::new(config);
        let target = Vector3::new(1000.0, 0.0, 50.0);
        let signal_dbm = -70.0;
        let noise_dbm = -100.0;
        let packet_length = 1000; // bits

        let per = jammer.compute_packet_error_rate(signal_dbm, target, noise_dbm, packet_length);

        // Should have some packet errors under jamming
        assert!(per > 0.0);
        assert!(per <= 1.0);
    }
}
