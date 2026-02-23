//! Utility functions for RF calculations

use crate::constants::*;

/// Convert power from Watts to dBm
pub fn watts_to_dbm(watts: f64) -> f64 {
    10.0 * (watts / 0.001).log10()
}

/// Convert power from dBm to Watts
pub fn dbm_to_watts(dbm: f64) -> f64 {
    0.001 * 10.0_f64.powf(dbm / 10.0)
}

/// Convert power from Watts to dBW
pub fn watts_to_dbw(watts: f64) -> f64 {
    10.0 * watts.log10()
}

/// Convert power from dBW to Watts
pub fn dbw_to_watts(dbw: f64) -> f64 {
    10.0_f64.powf(dbw / 10.0)
}

/// Calculate free-space path loss (Friis formula)
pub fn friis_path_loss(distance_m: f64, frequency_hz: f64) -> f64 {
    let wavelength = SPEED_OF_LIGHT / frequency_hz;
    20.0 * (4.0 * std::f64::consts::PI * distance_m / wavelength).log10()
}

/// Convert dB (ratio) to linear scale
#[inline]
pub fn db_to_linear(db: f64) -> f64 {
    10.0_f64.powf(db / 10.0)
}

/// Convert linear scale to dB (ratio)
#[inline]
pub fn linear_to_db(linear: f64) -> f64 {
    10.0 * linear.log10()
}

/// Calculate wavelength from frequency
pub fn wavelength(frequency_hz: f64) -> f64 {
    SPEED_OF_LIGHT / frequency_hz
}

/// Calculate thermal noise power
pub fn thermal_noise_power(temperature_k: f64, bandwidth_hz: f64) -> f64 {
    BOLTZMANN_CONSTANT * temperature_k * bandwidth_hz
}

/// Calculate thermal noise power in dBm
pub fn thermal_noise_dbm(temperature_k: f64, bandwidth_hz: f64) -> f64 {
    let noise_watts = thermal_noise_power(temperature_k, bandwidth_hz);
    watts_to_dbm(noise_watts)
}

/// Calculate SNR (Signal-to-Noise Ratio)
pub fn calculate_snr(signal_dbm: f64, noise_dbm: f64) -> f64 {
    signal_dbm - noise_dbm
}

/// Calculate link budget
pub fn link_budget(
    tx_power_dbm: f64,
    tx_gain_dbi: f64,
    rx_gain_dbi: f64,
    path_loss_db: f64,
    system_loss_db: f64,
) -> f64 {
    tx_power_dbm + tx_gain_dbi + rx_gain_dbi - path_loss_db - system_loss_db
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_power_conversions() {
        let watts = 1.0;
        let dbm = watts_to_dbm(watts);
        assert!((dbm - 30.0).abs() < 0.001);

        let converted_watts = dbm_to_watts(dbm);
        assert!((converted_watts - watts).abs() < 0.001);
    }

    #[test]
    fn test_wavelength() {
        let freq = 2.4e9; // 2.4 GHz
        let lambda = wavelength(freq);
        assert!((lambda - 0.125).abs() < 0.001);
    }

    #[test]
    fn test_thermal_noise() {
        let temp = 290.0; // Room temperature (K)
        let bandwidth = 1e6; // 1 MHz
        let noise = thermal_noise_dbm(temp, bandwidth);

        // Should be around -114 dBm for 1 MHz at 290K
        assert!(noise > -115.0 && noise < -113.0);
    }

    #[test]
    fn test_link_budget() {
        let budget = link_budget(
            20.0, // 20 dBm Tx power
            3.0,  // 3 dBi Tx gain
            3.0,  // 3 dBi Rx gain
            80.0, // 80 dB path loss
            2.0,  // 2 dB system losses
        );

        // Should be around -56 dBm
        assert!((budget - (-56.0)).abs() < 0.1);
    }
}
