//! Antenna models and patterns

use nalgebra::Vector3;
use serde::{Deserialize, Serialize};

/// Antenna polarization type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum PolarizationType {
    /// Linear vertical polarization
    Vertical,
    /// Linear horizontal polarization
    Horizontal,
    /// Circular polarization (right-hand)
    CircularRight,
    /// Circular polarization (left-hand)
    CircularLeft,
    /// Elliptical polarization
    Elliptical,
}

/// Antenna radiation pattern
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AntennaPattern {
    /// Omnidirectional (isotropic)
    Isotropic,
    /// Dipole pattern
    Dipole,
    /// Directional with beamwidth
    Directional {
        azimuth_beamwidth_deg: f64,
        elevation_beamwidth_deg: f64,
        front_to_back_ratio_db: f64,
    },
    /// Custom pattern from measurements
    Custom {
        azimuth_pattern: Vec<f64>,
        elevation_pattern: Vec<f64>,
    },
}

impl AntennaPattern {
    /// Calculate gain in a given direction
    pub fn gain(&self, direction: Vector3<f64>, main_direction: Vector3<f64>) -> f64 {
        match self {
            AntennaPattern::Isotropic => 0.0, // 0 dBi
            AntennaPattern::Dipole => self.dipole_gain(direction, main_direction),
            AntennaPattern::Directional {
                azimuth_beamwidth_deg,
                elevation_beamwidth_deg,
                front_to_back_ratio_db,
            } => self.directional_gain(
                direction,
                main_direction,
                *azimuth_beamwidth_deg,
                *elevation_beamwidth_deg,
                *front_to_back_ratio_db,
            ),
            AntennaPattern::Custom { .. } => {
                // TODO: Implement custom pattern interpolation
                0.0
            }
        }
    }

    fn dipole_gain(&self, direction: Vector3<f64>, main_direction: Vector3<f64>) -> f64 {
        let cos_theta = direction.normalize().dot(&main_direction.normalize());
        let theta = cos_theta.acos();

        // Dipole pattern: sin^2(theta)
        let pattern_value = theta.sin().powi(2);

        // Convert to dB (dipole has 2.15 dBi max gain)
        2.15 + 10.0 * pattern_value.log10()
    }

    fn directional_gain(
        &self,
        direction: Vector3<f64>,
        main_direction: Vector3<f64>,
        azimuth_beamwidth: f64,
        elevation_beamwidth: f64,
        front_to_back_ratio: f64,
    ) -> f64 {
        let dir_norm = direction.normalize();
        let main_norm = main_direction.normalize();

        let cos_angle = dir_norm.dot(&main_norm);
        let angle_deg = cos_angle.acos().to_degrees();

        // Gaussian approximation for beamwidth
        let beamwidth = (azimuth_beamwidth + elevation_beamwidth) / 2.0;
        let half_power_angle = beamwidth / 2.0;

        if angle_deg < 90.0 {
            // Front hemisphere
            let attenuation = -12.0 * (angle_deg / half_power_angle).powi(2);
            attenuation.max(-front_to_back_ratio)
        } else {
            // Back hemisphere
            -front_to_back_ratio
        }
    }
}

/// Antenna model
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Antenna {
    pub name: String,
    pub pattern: AntennaPattern,
    pub polarization: PolarizationType,
    pub gain_dbi: f64,
    pub efficiency: f64,
    pub direction: Vector3<f64>,
}

impl Antenna {
    /// Create an isotropic antenna
    pub fn isotropic() -> Self {
        Self {
            name: "Isotropic".to_string(),
            pattern: AntennaPattern::Isotropic,
            polarization: PolarizationType::Vertical,
            gain_dbi: 0.0,
            efficiency: 1.0,
            direction: Vector3::new(0.0, 0.0, 1.0),
        }
    }

    /// Create a dipole antenna
    pub fn dipole() -> Self {
        Self {
            name: "Dipole".to_string(),
            pattern: AntennaPattern::Dipole,
            polarization: PolarizationType::Vertical,
            gain_dbi: 2.15,
            efficiency: 0.95,
            direction: Vector3::new(0.0, 0.0, 1.0),
        }
    }

    /// Create a directional antenna
    pub fn directional(azimuth_beamwidth: f64, elevation_beamwidth: f64) -> Self {
        Self {
            name: "Directional".to_string(),
            pattern: AntennaPattern::Directional {
                azimuth_beamwidth_deg: azimuth_beamwidth,
                elevation_beamwidth_deg: elevation_beamwidth,
                front_to_back_ratio_db: 20.0,
            },
            polarization: PolarizationType::Vertical,
            gain_dbi: 10.0,
            efficiency: 0.90,
            direction: Vector3::new(0.0, 0.0, 1.0),
        }
    }

    /// Calculate effective gain in a given direction
    pub fn effective_gain(&self, direction: Vector3<f64>) -> f64 {
        let pattern_gain = self.pattern.gain(direction, self.direction);
        self.gain_dbi + pattern_gain
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_isotropic_antenna() {
        let ant = Antenna::isotropic();
        assert_eq!(ant.gain_dbi, 0.0);

        let direction = Vector3::new(1.0, 0.0, 0.0);
        let gain = ant.effective_gain(direction);
        assert!((gain - 0.0).abs() < 0.01);
    }

    #[test]
    fn test_dipole_antenna() {
        let ant = Antenna::dipole();
        assert_eq!(ant.gain_dbi, 2.15);
    }

    #[test]
    fn test_directional_antenna() {
        let ant = Antenna::directional(60.0, 60.0);
        assert!(ant.gain_dbi > 5.0);
    }
}
