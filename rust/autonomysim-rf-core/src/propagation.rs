//! RF propagation engine and configuration

use crate::constants::SPEED_OF_LIGHT;
use autonomysim_core::prelude::*;
use nalgebra::Point3;
use serde::{Deserialize, Serialize};
use std::sync::Arc;
use thiserror::Error;

/// Result type for RF operations
pub type RFResult<T> = Result<T, RFError>;

/// Errors that can occur during RF propagation
#[derive(Error, Debug)]
pub enum RFError {
    #[error("Simulation error: {0}")]
    SimulationError(#[from] SimError),

    #[error("Invalid frequency: {0}")]
    InvalidFrequency(String),

    #[error("Invalid power: {0}")]
    InvalidPower(String),

    #[error("Computation error: {0}")]
    ComputationError(String),
}

/// RF propagation model selection
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum PropagationModel {
    /// Free-space path loss (Friis equation)
    Friis,
    /// Two-ray ground reflection model
    TwoRay,
    /// Log-distance path loss model
    LogDistance,
    /// Irregular Terrain Model (Longley-Rice)
    ITM,
    /// Physics-based ray tracing
    RayTracing,
    /// Gaussian beam propagation
    GaussianBeam,
    /// COST 231 model (urban)
    COST231,
}

impl PropagationModel {
    pub fn as_str(&self) -> &str {
        match self {
            PropagationModel::Friis => "friis",
            PropagationModel::TwoRay => "two_ray",
            PropagationModel::LogDistance => "log_distance",
            PropagationModel::ITM => "itm",
            PropagationModel::RayTracing => "ray_tracing",
            PropagationModel::GaussianBeam => "gaussian_beam",
            PropagationModel::COST231 => "cost231",
        }
    }
}

/// Configuration for RF propagation simulation
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PropagationConfig {
    /// Propagation model to use
    pub model: PropagationModel,

    /// Carrier frequency in Hz
    pub frequency_hz: f64,

    /// Transmitter power in dBm
    pub tx_power_dbm: f64,

    /// Transmitter antenna gain in dBi
    pub tx_gain_dbi: f64,

    /// Receiver antenna gain in dBi
    pub rx_gain_dbi: f64,

    /// System losses in dB
    pub system_loss_db: f64,

    /// Number of rays for ray tracing
    pub num_rays: usize,

    /// Maximum number of reflections
    pub max_reflections: usize,

    /// Maximum ray distance in meters
    pub max_distance: f64,

    /// Log-distance path loss exponent
    pub path_loss_exponent: f64,

    /// Reference distance for log-distance model (meters)
    pub reference_distance: f64,

    /// Enable diffraction calculation
    pub enable_diffraction: bool,

    /// Enable scattering calculation
    pub enable_scattering: bool,

    /// Thermal noise floor in dBm/Hz
    pub noise_floor_dbm_hz: f64,
}

impl Default for PropagationConfig {
    fn default() -> Self {
        Self {
            model: PropagationModel::RayTracing,
            frequency_hz: 2.4e9,
            tx_power_dbm: 20.0,
            tx_gain_dbi: 0.0,
            rx_gain_dbi: 0.0,
            system_loss_db: 0.0,
            num_rays: 1000,
            max_reflections: 5,
            max_distance: 1000.0,
            path_loss_exponent: 2.0,
            reference_distance: 1.0,
            enable_diffraction: true,
            enable_scattering: false,
            noise_floor_dbm_hz: -174.0, // Thermal noise at room temperature
        }
    }
}

impl PropagationConfig {
    /// Calculate wavelength from frequency
    pub fn wavelength(&self) -> f64 {
        SPEED_OF_LIGHT / self.frequency_hz
    }

    /// Validate configuration
    pub fn validate(&self) -> RFResult<()> {
        if self.frequency_hz <= 0.0 {
            return Err(RFError::InvalidFrequency(
                "Frequency must be positive".to_string(),
            ));
        }

        if self.num_rays == 0 {
            return Err(RFError::ComputationError(
                "Number of rays must be greater than zero".to_string(),
            ));
        }

        if self.max_distance <= 0.0 {
            return Err(RFError::ComputationError(
                "Maximum distance must be positive".to_string(),
            ));
        }

        Ok(())
    }
}

/// Main RF propagation engine
pub struct RFPropagationEngine {
    backend: Arc<dyn SimulationBackend>,
    config: PropagationConfig,
    scene: Option<SceneHandle>,
}

impl RFPropagationEngine {
    /// Create a new RF propagation engine
    pub fn new(backend: Arc<dyn SimulationBackend>, config: PropagationConfig) -> Self {
        Self {
            backend,
            config,
            scene: None,
        }
    }

    /// Set the scene to use for propagation
    pub fn set_scene(&mut self, scene: SceneHandle) {
        self.scene = Some(scene);
    }

    /// Get the current configuration
    pub fn config(&self) -> &PropagationConfig {
        &self.config
    }

    /// Update configuration
    pub fn set_config(&mut self, config: PropagationConfig) {
        self.config = config;
    }

    /// Compute path loss between transmitter and receiver
    pub async fn compute_path_loss(
        &self,
        tx_pos: Point3<f64>,
        rx_pos: Point3<f64>,
    ) -> RFResult<f64> {
        self.config.validate()?;

        let distance = (tx_pos - rx_pos).norm();

        match self.config.model {
            PropagationModel::Friis => Ok(self.friis_path_loss(distance)),
            PropagationModel::TwoRay => Ok(self.two_ray_path_loss(distance, tx_pos.z, rx_pos.z)),
            PropagationModel::LogDistance => Ok(self.log_distance_path_loss(distance)),
            PropagationModel::RayTracing => self.ray_tracing_path_loss(tx_pos, rx_pos).await,
            PropagationModel::GaussianBeam => Ok(self.gaussian_beam_path_loss(distance)),
            PropagationModel::ITM => Ok(self.itm_path_loss(distance)),
            PropagationModel::COST231 => Ok(self.cost231_path_loss(distance)),
        }
    }

    /// Compute received signal strength indicator (RSSI)
    pub async fn compute_rssi(&self, tx_pos: Point3<f64>, rx_pos: Point3<f64>) -> RFResult<f64> {
        let path_loss = self.compute_path_loss(tx_pos, rx_pos).await?;

        // RSSI = Tx Power + Tx Gain + Rx Gain - Path Loss - System Losses
        let rssi = self.config.tx_power_dbm + self.config.tx_gain_dbi + self.config.rx_gain_dbi
            - path_loss
            - self.config.system_loss_db;

        Ok(rssi)
    }

    /// Friis free-space path loss
    fn friis_path_loss(&self, distance: f64) -> f64 {
        let wavelength = self.config.wavelength();
        20.0 * (4.0 * std::f64::consts::PI * distance / wavelength).log10()
    }

    /// Two-ray ground reflection model
    fn two_ray_path_loss(&self, distance: f64, tx_height: f64, rx_height: f64) -> f64 {
        if distance < 1.0 {
            return self.friis_path_loss(distance);
        }

        // Simplified two-ray model
        40.0 * distance.log10() - (10.0 * tx_height.log10() + 10.0 * rx_height.log10())
    }

    /// Log-distance path loss model
    fn log_distance_path_loss(&self, distance: f64) -> f64 {
        let d0 = self.config.reference_distance;
        let pl0 = self.friis_path_loss(d0);

        pl0 + 10.0 * self.config.path_loss_exponent * (distance / d0).log10()
    }

    /// Ray tracing path loss (uses backend for scene queries)
    async fn ray_tracing_path_loss(
        &self,
        tx_pos: Point3<f64>,
        rx_pos: Point3<f64>,
    ) -> RFResult<f64> {
        let scene = self
            .scene
            .as_ref()
            .ok_or_else(|| RFError::ComputationError("No scene set for ray tracing".to_string()))?;

        // Direct ray
        let direction = (rx_pos - tx_pos).normalize();
        let distance = (tx_pos - rx_pos).norm();

        let ray = Ray {
            origin: tx_pos,
            direction,
            max_distance: distance,
        };

        // Check for line-of-sight
        let hit = self.backend.cast_ray(scene, &ray)?;

        if hit.is_none() {
            // Line of sight - use Friis
            return Ok(self.friis_path_loss(distance));
        }

        // Obstruction detected - add additional loss
        let base_loss = self.friis_path_loss(distance);
        let obstruction_loss = 20.0; // Simplified - could be more sophisticated

        Ok(base_loss + obstruction_loss)
    }

    /// Gaussian beam path loss (simplified)
    fn gaussian_beam_path_loss(&self, distance: f64) -> f64 {
        let wavelength = self.config.wavelength();
        let beam_waist = wavelength; // Simplified

        let rayleigh_range = std::f64::consts::PI * beam_waist.powi(2) / wavelength;
        let beam_spread = (1.0 + (distance / rayleigh_range).powi(2)).sqrt();

        self.friis_path_loss(distance) + 10.0 * beam_spread.log10()
    }

    /// ITM (Longley-Rice) model (simplified)
    fn itm_path_loss(&self, distance: f64) -> f64 {
        // Simplified version - full ITM is very complex
        let base_loss = self.log_distance_path_loss(distance);

        // Add terrain effects (simplified)
        let terrain_loss = 5.0 * (distance / 1000.0).sqrt();

        base_loss + terrain_loss
    }

    /// COST 231 model for urban environments
    fn cost231_path_loss(&self, distance: f64) -> f64 {
        let freq_mhz = self.config.frequency_hz / 1e6;

        // COST 231 Hata model (simplified)
        46.3 + 33.9 * freq_mhz.log10() - 13.82 * 30.0_f64.log10() // Assume 30m antenna height
            + (44.9 - 6.55 * 30.0_f64.log10()) * (distance / 1000.0).log10()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use autonomysim_core::native::NativeBackend;

    #[tokio::test]
    async fn test_friis_path_loss() {
        let config = PropagationConfig {
            model: PropagationModel::Friis,
            frequency_hz: 2.4e9,
            ..Default::default()
        };

        let backend = Arc::new(NativeBackend::new());
        let engine = RFPropagationEngine::new(backend, config);

        let tx = Point3::new(0.0, 0.0, 0.0);
        let rx = Point3::new(100.0, 0.0, 0.0);

        let path_loss = engine.compute_path_loss(tx, rx).await.unwrap();
        assert!(path_loss > 0.0);
        assert!(path_loss < 200.0); // Reasonable range
    }

    #[test]
    fn test_wavelength() {
        let config = PropagationConfig {
            frequency_hz: 2.4e9,
            ..Default::default()
        };

        let wavelength = config.wavelength();
        assert!((wavelength - 0.125).abs() < 0.001);
    }
}
