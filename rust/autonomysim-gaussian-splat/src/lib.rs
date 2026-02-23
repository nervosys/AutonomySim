//! # 3D Gaussian Splatting for RF Fields
//!
//! Neural representation of radio frequency fields using 3D Gaussian splatting.
//!
//! ## Overview
//!
//! This crate implements 3D Gaussian splatting for representing RF propagation
//! as a continuous neural field. Instead of discrete ray tracing or grid-based
//! methods, signal strength is represented as a weighted sum of 3D Gaussians.
//!
//! ## Key Features
//!
//! - **Neural RF Fields**: Continuous representation of signal strength
//! - **Fast Queries**: O(N) evaluation where N is number of Gaussians (typically <1000)
//! - **Training**: Learn from measurements or ray tracing data
//! - **Compact**: Far smaller than voxel grids
//! - **Differentiable**: Can be used in optimization pipelines
//!
//! ## Architecture
//!
//! ```text
//! Measurements       Training         Gaussian Field      Query
//! (RSSI, position)  ────────>  [Gaussian Centers]  ────>  Signal
//!                              [Covariances]              Strength
//!                              [Amplitudes]
//! ```
//!
//! ## Example
//!
//! ```rust
//! use autonomysim_gaussian_splat::{GaussianRFField, RFMeasurement, TrainingConfig};
//! use nalgebra::Vector3;
//! use std::f64::consts::PI;
//!
//! // Create measurements (need enough for training)
//! let measurements: Vec<RFMeasurement> = (0..150)
//!     .map(|i| {
//!         let angle = (i as f64 / 150.0) * 2.0 * PI;
//!         let radius = 5.0 + (i as f64 / 10.0);
//!         let pos = Vector3::new(radius * angle.cos(), radius * angle.sin(), 0.0);
//!         let distance = pos.norm().max(1.0);
//!         let rssi = -30.0 - 20.0 * distance.log10();
//!         RFMeasurement::new(pos, rssi)
//!     })
//!     .collect();
//!
//! // Train Gaussian field with fewer Gaussians
//! let mut field = GaussianRFField::new();
//! let config = TrainingConfig {
//!     num_gaussians: 20,
//!     max_iterations: 50,
//!     ..Default::default()
//! };
//! field.train(&measurements, config).unwrap();
//!
//! // Query signal strength
//! let position = Vector3::new(5.0, 0.0, 0.0);
//! let rssi = field.query(position);
//! println!("Signal strength at {:?}: {:.1} dBm", position, rssi);
//! ```

use nalgebra::{Matrix3, Vector3};
use rayon::prelude::*;
use serde::{Deserialize, Serialize};
use std::f64::consts::PI;

/// Error types for Gaussian splatting operations
#[derive(Debug, Clone)]
pub enum GaussianError {
    /// Training failed to converge
    ConvergenceFailure(String),
    /// Invalid configuration
    InvalidConfig(String),
    /// Insufficient data
    InsufficientData(String),
    /// Numerical error (NaN, infinity)
    NumericalError(String),
}

impl std::fmt::Display for GaussianError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::ConvergenceFailure(msg) => write!(f, "Convergence failure: {}", msg),
            Self::InvalidConfig(msg) => write!(f, "Invalid config: {}", msg),
            Self::InsufficientData(msg) => write!(f, "Insufficient data: {}", msg),
            Self::NumericalError(msg) => write!(f, "Numerical error: {}", msg),
        }
    }
}

impl std::error::Error for GaussianError {}

pub type GaussianResult<T> = Result<T, GaussianError>;

/// RF measurement (position, RSSI)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RFMeasurement {
    /// 3D position where measurement was taken
    pub position: Vector3<f64>,
    /// Received signal strength indicator (dBm)
    pub rssi_dbm: f64,
    /// Optional: measurement weight (default: 1.0)
    pub weight: f64,
}

impl RFMeasurement {
    /// Create a new RF measurement
    pub fn new(position: Vector3<f64>, rssi_dbm: f64) -> Self {
        Self {
            position,
            rssi_dbm,
            weight: 1.0,
        }
    }

    /// Create a weighted measurement
    pub fn with_weight(position: Vector3<f64>, rssi_dbm: f64, weight: f64) -> Self {
        Self {
            position,
            rssi_dbm,
            weight,
        }
    }
}

/// 3D Gaussian primitive
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Gaussian3D {
    /// Center position in 3D space
    pub center: Vector3<f64>,

    /// Covariance matrix (3×3 symmetric positive-definite)
    pub covariance: Matrix3<f64>,

    /// Inverse covariance (cached for fast evaluation)
    #[serde(skip)]
    pub inv_covariance: Option<Matrix3<f64>>,

    /// Amplitude (signal strength contribution in dBm)
    pub amplitude: f64,

    /// Normalization factor (cached)
    #[serde(skip)]
    pub normalization: Option<f64>,
}

impl Gaussian3D {
    /// Create a new isotropic Gaussian
    pub fn new(center: Vector3<f64>, variance: f64, amplitude: f64) -> Self {
        let covariance = Matrix3::from_diagonal(&Vector3::new(variance, variance, variance));
        Self {
            center,
            covariance,
            inv_covariance: None,
            amplitude,
            normalization: None,
        }
    }

    /// Create an anisotropic Gaussian with custom covariance
    pub fn with_covariance(center: Vector3<f64>, covariance: Matrix3<f64>, amplitude: f64) -> Self {
        Self {
            center,
            covariance,
            inv_covariance: None,
            amplitude,
            normalization: None,
        }
    }

    /// Precompute cached values for fast evaluation
    pub fn precompute(&mut self) -> GaussianResult<()> {
        // Compute inverse covariance
        self.inv_covariance = self.covariance.try_inverse();

        if self.inv_covariance.is_none() {
            return Err(GaussianError::NumericalError(
                "Covariance matrix is singular".to_string(),
            ));
        }

        // Compute normalization factor: 1 / sqrt((2π)^3 * det(Σ))
        let det = self.covariance.determinant();
        if det <= 0.0 {
            return Err(GaussianError::NumericalError(format!(
                "Invalid covariance determinant: {}",
                det
            )));
        }

        self.normalization = Some(1.0 / ((2.0 * PI).powf(1.5) * det.sqrt()));

        Ok(())
    }

    /// Evaluate Gaussian at a given position
    pub fn evaluate(&self, position: Vector3<f64>) -> f64 {
        let delta = position - self.center;

        let inv_cov = self
            .inv_covariance
            .as_ref()
            .expect("Call precompute() before evaluate()");

        // Mahalanobis distance: (x - μ)ᵀ Σ⁻¹ (x - μ)
        let exponent = -0.5 * (delta.transpose() * inv_cov * delta)[0];

        let norm = self
            .normalization
            .expect("Call precompute() before evaluate()");

        // Gaussian value scaled by amplitude
        self.amplitude * norm * exponent.exp()
    }

    /// Evaluate Gaussian contribution (without normalization, for speed)
    pub fn evaluate_fast(&self, position: Vector3<f64>) -> f64 {
        let delta = position - self.center;

        let inv_cov = self
            .inv_covariance
            .as_ref()
            .expect("Call precompute() before evaluate_fast()");

        // Mahalanobis distance
        let exponent = -0.5 * (delta.transpose() * inv_cov * delta)[0];

        // Skip normalization for speed (relative values only)
        self.amplitude * exponent.exp()
    }
}

/// Training configuration for Gaussian RF field
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrainingConfig {
    /// Number of Gaussians to use
    pub num_gaussians: usize,

    /// Maximum training iterations
    pub max_iterations: usize,

    /// Convergence threshold (change in loss)
    pub convergence_threshold: f64,

    /// Learning rate for gradient descent
    pub learning_rate: f64,

    /// Initial Gaussian variance
    pub initial_variance: f64,

    /// Regularization weight (prevents overfitting)
    pub regularization: f64,

    /// Use parallel training
    pub parallel: bool,
}

impl Default for TrainingConfig {
    fn default() -> Self {
        Self {
            num_gaussians: 100,
            max_iterations: 1000,
            convergence_threshold: 1e-4,
            learning_rate: 0.01,
            initial_variance: 25.0, // 5m radius in free space
            regularization: 0.001,
            parallel: true,
        }
    }
}

/// 3D Gaussian RF field representation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GaussianRFField {
    /// Collection of 3D Gaussians
    pub gaussians: Vec<Gaussian3D>,

    /// Transmitter position (reference point)
    pub transmitter_pos: Option<Vector3<f64>>,

    /// Training statistics
    pub training_loss: Option<f64>,
    pub training_iterations: Option<usize>,
}

impl GaussianRFField {
    /// Create an empty Gaussian RF field
    pub fn new() -> Self {
        Self {
            gaussians: Vec::new(),
            transmitter_pos: None,
            training_loss: None,
            training_iterations: None,
        }
    }

    /// Train Gaussian field from measurements
    pub fn train(
        &mut self,
        measurements: &[RFMeasurement],
        config: TrainingConfig,
    ) -> GaussianResult<()> {
        if measurements.len() < config.num_gaussians {
            return Err(GaussianError::InsufficientData(format!(
                "Need at least {} measurements for {} Gaussians",
                config.num_gaussians, config.num_gaussians
            )));
        }

        // Initialize Gaussians using K-means clustering
        self.initialize_gaussians(measurements, &config)?;

        // Precompute all Gaussians
        for gaussian in &mut self.gaussians {
            gaussian.precompute()?;
        }

        // Optimize Gaussian parameters
        let loss = self.optimize(measurements, &config)?;

        self.training_loss = Some(loss);
        self.training_iterations = Some(config.max_iterations);

        Ok(())
    }

    /// Initialize Gaussians using K-means clustering on measurement positions
    fn initialize_gaussians(
        &mut self,
        measurements: &[RFMeasurement],
        config: &TrainingConfig,
    ) -> GaussianResult<()> {
        // Simple K-means initialization
        let n = measurements.len();
        let k = config.num_gaussians;

        // Initialize centers randomly from measurements
        let mut centers: Vec<Vector3<f64>> =
            (0..k).map(|i| measurements[i * n / k].position).collect();

        // K-means iterations
        for _ in 0..10 {
            // Assign measurements to nearest center
            let mut clusters: Vec<Vec<&RFMeasurement>> = vec![Vec::new(); k];

            for measurement in measurements {
                let mut min_dist = f64::INFINITY;
                let mut cluster_idx = 0;

                for (idx, center) in centers.iter().enumerate() {
                    let dist = (measurement.position - center).norm();
                    if dist < min_dist {
                        min_dist = dist;
                        cluster_idx = idx;
                    }
                }

                clusters[cluster_idx].push(measurement);
            }

            // Update centers
            for (idx, cluster) in clusters.iter().enumerate() {
                if !cluster.is_empty() {
                    let sum: Vector3<f64> = cluster
                        .iter()
                        .map(|m| m.position)
                        .fold(Vector3::zeros(), |acc, pos| acc + pos);
                    centers[idx] = sum / cluster.len() as f64;
                }
            }
        }

        // Create Gaussians from cluster centers
        self.gaussians = centers
            .into_iter()
            .map(|center| {
                // Estimate amplitude from nearby measurements
                let nearby: Vec<_> = measurements
                    .iter()
                    .filter(|m| (m.position - center).norm() < config.initial_variance.sqrt() * 2.0)
                    .collect();

                let amplitude = if !nearby.is_empty() {
                    nearby.iter().map(|m| m.rssi_dbm).sum::<f64>() / nearby.len() as f64
                } else {
                    -70.0 // Default
                };

                Gaussian3D::new(center, config.initial_variance, amplitude)
            })
            .collect();

        Ok(())
    }

    /// Optimize Gaussian parameters using gradient descent
    fn optimize(
        &mut self,
        measurements: &[RFMeasurement],
        config: &TrainingConfig,
    ) -> GaussianResult<f64> {
        let mut prev_loss = f64::INFINITY;

        for _iteration in 0..config.max_iterations {
            // Compute current loss
            let loss = self.compute_loss(measurements, config.regularization);

            // Check convergence
            if (prev_loss - loss).abs() < config.convergence_threshold {
                return Ok(loss);
            }

            // Compute gradients for all Gaussians first
            let gradients: Vec<f64> = self
                .gaussians
                .iter()
                .map(|g| Self::compute_amplitude_gradient_static(g, &self.gaussians, measurements))
                .collect();

            // Apply gradient updates with clamping
            for (gaussian, gradient) in self.gaussians.iter_mut().zip(gradients.iter()) {
                // Clamp gradient to prevent numerical instability
                let clamped_gradient = gradient.clamp(-100.0, 100.0);
                gaussian.amplitude -= config.learning_rate * clamped_gradient;

                // Clamp amplitude to reasonable RF range (-150 to +50 dBm)
                gaussian.amplitude = gaussian.amplitude.clamp(-150.0, 50.0);
            }

            // Recompute cached values
            for gaussian in &mut self.gaussians {
                gaussian.precompute()?;
            }

            prev_loss = loss;
        }

        Ok(prev_loss)
    }

    /// Compute training loss (MSE)
    fn compute_loss(&self, measurements: &[RFMeasurement], regularization: f64) -> f64 {
        let prediction_error: f64 = measurements
            .iter()
            .map(|m| {
                let predicted = self.query(m.position);
                let error = predicted - m.rssi_dbm;
                m.weight * error * error
            })
            .sum();

        // L2 regularization on amplitudes
        let reg_term: f64 = self
            .gaussians
            .iter()
            .map(|g| g.amplitude * g.amplitude)
            .sum();

        (prediction_error / measurements.len() as f64) + regularization * reg_term
    }

    /// Compute gradient of loss w.r.t. Gaussian amplitude (static version for optimization)
    fn compute_amplitude_gradient_static(
        target_gaussian: &Gaussian3D,
        all_gaussians: &[Gaussian3D],
        measurements: &[RFMeasurement],
    ) -> f64 {
        measurements
            .iter()
            .map(|m| {
                // Compute predicted value from all Gaussians
                let predicted: f64 = all_gaussians
                    .iter()
                    .map(|g| g.evaluate_fast(m.position))
                    .sum();

                let error = predicted - m.rssi_dbm;
                let contribution = target_gaussian.evaluate_fast(m.position);
                2.0 * m.weight * error * contribution / measurements.len() as f64
            })
            .sum()
    }

    /// Query signal strength at a position
    pub fn query(&self, position: Vector3<f64>) -> f64 {
        if self.gaussians.is_empty() {
            return -120.0; // Very weak signal
        }

        // Sum contributions from all Gaussians (weighted sum of Gaussians)
        let sum: f64 = self
            .gaussians
            .iter()
            .map(|g| g.evaluate_fast(position))
            .sum();

        // Clamp to reasonable range
        sum.clamp(-150.0, 50.0)
    }

    /// Query signal strength at multiple positions (parallel)
    pub fn query_batch(&self, positions: &[Vector3<f64>]) -> Vec<f64> {
        positions.par_iter().map(|pos| self.query(*pos)).collect()
    }

    /// Get the number of Gaussians
    pub fn num_gaussians(&self) -> usize {
        self.gaussians.len()
    }

    /// Set transmitter position
    pub fn set_transmitter(&mut self, position: Vector3<f64>) {
        self.transmitter_pos = Some(position);
    }
}

impl Default for GaussianRFField {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_gaussian_creation() {
        let gaussian = Gaussian3D::new(Vector3::new(0.0, 0.0, 0.0), 25.0, -50.0);

        assert_eq!(gaussian.center, Vector3::new(0.0, 0.0, 0.0));
        assert_eq!(gaussian.amplitude, -50.0);
    }

    #[test]
    fn test_gaussian_evaluation() {
        let mut gaussian = Gaussian3D::new(Vector3::new(0.0, 0.0, 0.0), 1.0, 1.0);

        gaussian.precompute().unwrap();

        // At center, should be maximum
        let val_center = gaussian.evaluate(Vector3::new(0.0, 0.0, 0.0));

        // Away from center, should be smaller
        let val_away = gaussian.evaluate(Vector3::new(3.0, 0.0, 0.0));

        assert!(val_center > val_away);
    }

    #[test]
    fn test_rf_measurement() {
        let measurement = RFMeasurement::new(Vector3::new(10.0, 20.0, 30.0), -60.0);

        assert_eq!(measurement.position, Vector3::new(10.0, 20.0, 30.0));
        assert_eq!(measurement.rssi_dbm, -60.0);
        assert_eq!(measurement.weight, 1.0);
    }

    #[test]
    fn test_gaussian_field_creation() {
        let field = GaussianRFField::new();
        assert_eq!(field.num_gaussians(), 0);
    }

    #[test]
    fn test_training_config_default() {
        let config = TrainingConfig::default();
        assert_eq!(config.num_gaussians, 100);
        assert!(config.parallel);
    }

    #[test]
    fn test_field_training() {
        // Create synthetic measurements (simple distance-based falloff)
        let tx_pos = Vector3::new(0.0, 0.0, 0.0);
        let measurements: Vec<RFMeasurement> = (0..200)
            .map(|i| {
                let angle = (i as f64 / 200.0) * 2.0 * PI;
                let radius = 10.0 + (i as f64 / 20.0);
                let pos = Vector3::new(radius * angle.cos(), radius * angle.sin(), 0.0);

                // Simple path loss: -30dBm at 1m, -20dB/decade
                let distance = (pos - tx_pos).norm().max(1.0);
                let rssi = -30.0 - 20.0 * distance.log10();

                RFMeasurement::new(pos, rssi)
            })
            .collect();

        let mut field = GaussianRFField::new();
        field.set_transmitter(tx_pos);

        let config = TrainingConfig {
            num_gaussians: 20,
            max_iterations: 100,
            ..Default::default()
        };

        let result = field.train(&measurements, config);
        assert!(result.is_ok());
        assert_eq!(field.num_gaussians(), 20);
    }

    #[test]
    fn test_batch_query() {
        let mut field = GaussianRFField::new();

        // Add a few Gaussians manually
        let mut g1 = Gaussian3D::new(Vector3::new(0.0, 0.0, 0.0), 25.0, -40.0);
        g1.precompute().unwrap();

        let mut g2 = Gaussian3D::new(Vector3::new(10.0, 0.0, 0.0), 25.0, -60.0);
        g2.precompute().unwrap();

        field.gaussians = vec![g1, g2];

        let positions = vec![
            Vector3::new(0.0, 0.0, 0.0),
            Vector3::new(5.0, 0.0, 0.0),
            Vector3::new(10.0, 0.0, 0.0),
        ];

        let results = field.query_batch(&positions);
        assert_eq!(results.len(), 3);
    }
}
