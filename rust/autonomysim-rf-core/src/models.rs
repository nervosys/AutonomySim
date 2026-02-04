//! RF propagation models

use nalgebra::Vector3;
use serde::{Deserialize, Serialize};

/// Ray tube for ray tracing
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RayTube {
    pub origin: Vector3<f64>,
    pub direction: Vector3<f64>,
    pub power_dbm: f64,
    pub phase: f64,
    pub num_reflections: usize,
    pub path_length: f64,
}

impl RayTube {
    pub fn new(origin: Vector3<f64>, direction: Vector3<f64>, power_dbm: f64) -> Self {
        Self {
            origin,
            direction: direction.normalize(),
            power_dbm,
            phase: 0.0,
            num_reflections: 0,
            path_length: 0.0,
        }
    }
}

/// Fresnel reflection coefficient calculator
pub struct FresnelCalculator;

impl FresnelCalculator {
    /// Calculate reflection coefficient for parallel polarization
    pub fn reflection_parallel(
        incident_angle: f64,
        epsilon_r: f64,
        sigma: f64,
        frequency: f64,
    ) -> f64 {
        let cos_theta = incident_angle.cos();
        let sin_theta = incident_angle.sin();

        // Complex permittivity
        let epsilon_c = epsilon_r - (sigma / (2.0 * std::f64::consts::PI * frequency * 8.854e-12));

        let sqrt_term = (epsilon_c - sin_theta.powi(2)).sqrt();
        let numerator = epsilon_c * cos_theta - sqrt_term;
        let denominator = epsilon_c * cos_theta + sqrt_term;

        (numerator / denominator).abs()
    }

    /// Calculate reflection coefficient for perpendicular polarization
    pub fn reflection_perpendicular(
        incident_angle: f64,
        epsilon_r: f64,
        sigma: f64,
        frequency: f64,
    ) -> f64 {
        let cos_theta = incident_angle.cos();
        let sin_theta = incident_angle.sin();

        let epsilon_c = epsilon_r - (sigma / (2.0 * std::f64::consts::PI * frequency * 8.854e-12));

        let sqrt_term = (epsilon_c - sin_theta.powi(2)).sqrt();
        let numerator = cos_theta - sqrt_term;
        let denominator = cos_theta + sqrt_term;

        (numerator / denominator).abs()
    }
}

/// Knife-edge diffraction calculator
pub struct KnifeEdgeDiffraction;

impl KnifeEdgeDiffraction {
    /// Calculate diffraction loss for a knife edge obstruction
    pub fn calculate_loss(h: f64, d1: f64, d2: f64, wavelength: f64) -> f64 {
        // Fresnel-Kirchhoff parameter
        let v = h * (2.0 * (d1 + d2) / (wavelength * d1 * d2)).sqrt();

        if v <= -0.8 {
            // No significant diffraction
            0.0
        } else {
            // Bullington approximation
            let loss = 6.9 + 20.0 * ((v - 0.1).powi(2) + 1.0).sqrt().log10();
            loss.max(0.0)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ray_tube_creation() {
        let origin = Vector3::new(0.0, 0.0, 0.0);
        let direction = Vector3::new(1.0, 0.0, 0.0);
        let ray = RayTube::new(origin, direction, 20.0);

        assert_eq!(ray.power_dbm, 20.0);
        assert_eq!(ray.num_reflections, 0);
    }

    #[test]
    fn test_knife_edge_diffraction() {
        let loss = KnifeEdgeDiffraction::calculate_loss(
            10.0,  // obstruction height
            100.0, // distance to obstruction
            100.0, // distance from obstruction
            0.125, // wavelength (2.4 GHz)
        );

        assert!(loss > 0.0);
        assert!(loss < 50.0); // Reasonable range
    }
}
