//! Link Quality Metrics for Tactical Communications
//!
//! Implements:
//! - Signal-to-Noise Ratio (SNR) calculation
//! - Bit Error Rate (BER) estimation
//! - Packet Error Rate (PER) calculation
//! - Link budget analysis

use serde::{Deserialize, Serialize};
use std::f64::consts::PI;

/// Signal power metrics
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct SignalMetrics {
    /// Signal power (dBm)
    pub signal_power_dbm: f64,

    /// Noise power (dBm)
    pub noise_power_dbm: f64,

    /// Interference power (dBm)
    pub interference_power_dbm: f64,

    /// Signal-to-Noise Ratio (dB)
    pub snr_db: f64,

    /// Signal-to-Interference-plus-Noise Ratio (dB)
    pub sinr_db: f64,
}

impl SignalMetrics {
    /// Create new signal metrics
    pub fn new(signal_power_dbm: f64, noise_power_dbm: f64, interference_power_dbm: f64) -> Self {
        let snr_db = signal_power_dbm - noise_power_dbm;

        // SINR: S / (I + N)
        // Convert to linear scale
        let signal_w = dbm_to_watts(signal_power_dbm);
        let noise_w = dbm_to_watts(noise_power_dbm);
        let interference_w = dbm_to_watts(interference_power_dbm);

        let sinr = signal_w / (interference_w + noise_w);
        let sinr_db = watts_to_dbm(sinr);

        Self {
            signal_power_dbm,
            noise_power_dbm,
            interference_power_dbm,
            snr_db,
            sinr_db,
        }
    }

    /// Check if signal is above minimum threshold
    pub fn is_detectable(&self, min_snr_db: f64) -> bool {
        self.snr_db >= min_snr_db
    }
}

/// Bit Error Rate calculator for various modulation schemes
pub struct BERCalculator;

impl BERCalculator {
    /// Compute BER for BPSK modulation
    ///
    /// BER = 0.5 * erfc(sqrt(Eb/N0))
    ///
    /// Using approximation: erfc(x) ≈ exp(-x²) for x > 0
    pub fn bpsk(eb_n0_db: f64) -> f64 {
        let eb_n0 = db_to_linear(eb_n0_db);

        if eb_n0 > 20.0 {
            // Very high SNR, use asymptotic approximation
            1e-10
        } else if eb_n0 < 0.01 {
            // Very low SNR
            0.5
        } else {
            // Approximation: BER ≈ 0.5 * exp(-Eb/N0)
            0.5 * (-eb_n0).exp()
        }
    }

    /// Compute BER for QPSK modulation
    ///
    /// For QPSK, BER is same as BPSK when using Gray coding
    pub fn qpsk(eb_n0_db: f64) -> f64 {
        Self::bpsk(eb_n0_db)
    }

    /// Compute BER for 16-QAM modulation
    ///
    /// BER ≈ (3/8) * erfc(sqrt(Eb/N0 / 5))
    pub fn qam16(eb_n0_db: f64) -> f64 {
        let eb_n0 = db_to_linear(eb_n0_db);

        if eb_n0 > 20.0 {
            1e-9
        } else if eb_n0 < 0.1 {
            0.4
        } else {
            // Approximation
            0.375 * (-(eb_n0 / 5.0)).exp()
        }
    }

    /// Compute BER for 64-QAM modulation
    ///
    /// BER ≈ (7/24) * erfc(sqrt(Eb/N0 / 21))
    pub fn qam64(eb_n0_db: f64) -> f64 {
        let eb_n0 = db_to_linear(eb_n0_db);

        if eb_n0 > 25.0 {
            1e-8
        } else if eb_n0 < 1.0 {
            0.35
        } else {
            // Approximation
            0.29 * (-(eb_n0 / 21.0)).exp()
        }
    }

    /// Convert SNR to Eb/N0
    ///
    /// Eb/N0 = SNR * (Bandwidth / BitRate)
    pub fn snr_to_eb_n0(snr_db: f64, bandwidth_hz: f64, bit_rate_bps: f64) -> f64 {
        let bandwidth_factor = bandwidth_hz / bit_rate_bps;
        snr_db + 10.0 * bandwidth_factor.log10()
    }
}

/// Modulation scheme
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ModulationScheme {
    BPSK,
    QPSK,
    QAM16,
    QAM64,
}

impl ModulationScheme {
    /// Get bits per symbol
    pub fn bits_per_symbol(&self) -> usize {
        match self {
            Self::BPSK => 1,
            Self::QPSK => 2,
            Self::QAM16 => 4,
            Self::QAM64 => 6,
        }
    }

    /// Compute BER for this modulation
    pub fn compute_ber(&self, eb_n0_db: f64) -> f64 {
        match self {
            Self::BPSK => BERCalculator::bpsk(eb_n0_db),
            Self::QPSK => BERCalculator::qpsk(eb_n0_db),
            Self::QAM16 => BERCalculator::qam16(eb_n0_db),
            Self::QAM64 => BERCalculator::qam64(eb_n0_db),
        }
    }
}

/// Packet error rate calculator
pub struct PERCalculator;

impl PERCalculator {
    /// Compute PER from BER
    ///
    /// PER = 1 - (1 - BER)^N
    ///
    /// where N is packet length in bits
    pub fn from_ber(ber: f64, packet_length_bits: usize) -> f64 {
        if ber <= 0.0 {
            return 0.0;
        }

        if ber >= 1.0 {
            return 1.0;
        }

        // For small BER, use approximation: PER ≈ N * BER
        if ber < 1e-6 {
            (packet_length_bits as f64 * ber).min(1.0)
        } else {
            // Exact formula
            1.0 - (1.0 - ber).powi(packet_length_bits as i32)
        }
    }

    /// Compute PER from SINR for given modulation and packet size
    pub fn from_sinr(
        sinr_db: f64,
        modulation: ModulationScheme,
        bandwidth_hz: f64,
        bit_rate_bps: f64,
        packet_length_bits: usize,
    ) -> f64 {
        let eb_n0_db = BERCalculator::snr_to_eb_n0(sinr_db, bandwidth_hz, bit_rate_bps);
        let ber = modulation.compute_ber(eb_n0_db);
        Self::from_ber(ber, packet_length_bits)
    }
}

/// Link budget calculator
pub struct LinkBudget {
    /// Transmit power (dBm)
    pub tx_power_dbm: f64,

    /// Transmit antenna gain (dBi)
    pub tx_gain_dbi: f64,

    /// Receive antenna gain (dBi)
    pub rx_gain_dbi: f64,

    /// Free space path loss (dB)
    pub path_loss_db: f64,

    /// Additional losses (cable, polarization, etc.) (dB)
    pub additional_loss_db: f64,

    /// Noise figure (dB)
    pub noise_figure_db: f64,

    /// Bandwidth (Hz)
    pub bandwidth_hz: f64,
}

impl LinkBudget {
    /// Compute received signal power (dBm)
    ///
    /// Pr = Pt + Gt + Gr - Lpath - Ladditional
    pub fn received_power_dbm(&self) -> f64 {
        self.tx_power_dbm + self.tx_gain_dbi + self.rx_gain_dbi
            - self.path_loss_db
            - self.additional_loss_db
    }

    /// Compute noise power (dBm)
    ///
    /// N = k * T * B * NF
    ///
    /// where:
    /// - k = Boltzmann constant (1.38e-23 J/K)
    /// - T = Temperature (290 K standard)
    /// - B = Bandwidth (Hz)
    /// - NF = Noise figure (linear)
    pub fn noise_power_dbm(&self) -> f64 {
        const BOLTZMANN: f64 = 1.38e-23; // J/K
        const TEMP_K: f64 = 290.0; // Room temperature

        // Thermal noise power (dBm)
        let thermal_noise_dbm = 10.0 * ((BOLTZMANN * TEMP_K * self.bandwidth_hz) / 1e-3).log10();

        // Add noise figure
        thermal_noise_dbm + self.noise_figure_db
    }

    /// Compute SNR (dB)
    pub fn snr_db(&self) -> f64 {
        self.received_power_dbm() - self.noise_power_dbm()
    }

    /// Compute link margin (dB)
    ///
    /// Margin = SNR - Required_SNR
    pub fn link_margin_db(&self, required_snr_db: f64) -> f64 {
        self.snr_db() - required_snr_db
    }

    /// Check if link is viable
    pub fn is_viable(&self, required_snr_db: f64) -> bool {
        self.link_margin_db(required_snr_db) > 0.0
    }
}

/// Utility functions
#[inline]
pub fn dbm_to_watts(dbm: f64) -> f64 {
    10.0_f64.powf(dbm / 10.0) / 1000.0
}

#[inline]
pub fn watts_to_dbm(watts: f64) -> f64 {
    10.0 * (watts * 1000.0).log10()
}

#[inline]
pub fn db_to_linear(db: f64) -> f64 {
    10.0_f64.powf(db / 10.0)
}

#[inline]
pub fn linear_to_db(linear: f64) -> f64 {
    10.0 * linear.log10()
}

#[cfg(test)]
mod tests {
    use super::*;
    use approx::assert_abs_diff_eq;

    #[test]
    fn test_signal_metrics() {
        let metrics = SignalMetrics::new(-70.0, -100.0, -90.0);

        assert_abs_diff_eq!(metrics.snr_db, 30.0, epsilon = 0.1);
        assert!(metrics.is_detectable(10.0));
    }

    #[test]
    fn test_ber_bpsk() {
        // At high SNR, BER should be very low
        let ber_high = BERCalculator::bpsk(20.0);
        assert!(ber_high < 1e-5);

        // At low SNR, BER should approach 0.5
        let ber_low = BERCalculator::bpsk(-10.0);
        assert!(ber_low > 0.4);
    }

    #[test]
    fn test_per_calculation() {
        // With BER = 0.001 and 1000-bit packet
        let per = PERCalculator::from_ber(0.001, 1000);

        // PER ≈ 1 - (1 - 0.001)^1000 ≈ 0.63
        assert!(per > 0.6 && per < 0.7);
    }

    #[test]
    fn test_link_budget() {
        let budget = LinkBudget {
            tx_power_dbm: 30.0, // 1W
            tx_gain_dbi: 10.0,
            rx_gain_dbi: 10.0,
            path_loss_db: 100.0,
            additional_loss_db: 5.0,
            noise_figure_db: 3.0,
            bandwidth_hz: 20e6,
        };

        let rx_power = budget.received_power_dbm();
        assert_abs_diff_eq!(rx_power, -55.0, epsilon = 0.1);

        let snr = budget.snr_db();
        assert!(snr > 0.0); // Should have positive SNR
    }

    #[test]
    fn test_dbm_conversion() {
        let power_dbm = 30.0;
        let power_w = dbm_to_watts(power_dbm);
        let back_dbm = watts_to_dbm(power_w);

        assert_abs_diff_eq!(power_dbm, back_dbm, epsilon = 0.01);
        assert_abs_diff_eq!(power_w, 1.0, epsilon = 0.01); // 30 dBm = 1W
    }

    #[test]
    fn test_modulation_bits_per_symbol() {
        assert_eq!(ModulationScheme::BPSK.bits_per_symbol(), 1);
        assert_eq!(ModulationScheme::QPSK.bits_per_symbol(), 2);
        assert_eq!(ModulationScheme::QAM16.bits_per_symbol(), 4);
        assert_eq!(ModulationScheme::QAM64.bits_per_symbol(), 6);
    }
}
