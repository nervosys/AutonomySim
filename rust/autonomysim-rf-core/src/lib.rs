//! AutonomySim RF Propagation Core
//!
//! This crate provides RF (Radio Frequency) propagation models and utilities
//! for simulating wireless signal propagation in 3D environments.
//!
//! # Propagation Models
//!
//! The following models are supported:
//!
//! - **Friis**: Free-space path loss (line-of-sight)
//! - **Two-Ray Ground Reflection**: Ground reflection model
//! - **Log-Distance**: Empirical path loss model
//! - **ITM (Longley-Rice)**: Irregular terrain model
//! - **Ray Tracing**: Physics-based ray tracing with reflections/diffractions
//! - **Gaussian Beam**: Beam propagation method
//! - **COST 231**: Outdoor urban propagation
//!
//! # Example
//!
//! ```rust,no_run
//! use autonomysim_rf_core::prelude::*;
//! use autonomysim_core::prelude::*;
//! use autonomysim_core::native::NativeBackend;
//! use nalgebra::Point3;
//! use std::sync::Arc;
//!
//! #[tokio::main]
//! async fn main() -> anyhow::Result<()> {
//!     // Create backend and scene
//!     let mut backend = NativeBackend::new();
//!     backend.initialize(BackendConfig::default()).await?;
//!     let scene = backend.load_scene("city.obj").await?;
//!     
//!     // Create RF propagation engine
//!     let config = PropagationConfig {
//!         model: PropagationModel::Friis, // Use Friis for simplicity
//!         frequency_hz: 2.4e9,
//!         tx_power_dbm: 20.0,
//!         ..Default::default()
//!     };
//!     
//!     let backend_arc = Arc::new(backend);
//!     let mut engine = RFPropagationEngine::new(backend_arc, config);
//!     engine.set_scene(scene);
//!     
//!     // Compute path loss
//!     let tx_pos = Point3::new(0.0, 0.0, 10.0);
//!     let rx_pos = Point3::new(100.0, 0.0, 1.5);
//!     
//!     let path_loss = engine.compute_path_loss(tx_pos, rx_pos).await?;
//!     println!("Path loss: {:.2} dB", path_loss);
//!     
//!     // Get received signal strength
//!     let rssi = engine.compute_rssi(tx_pos, rx_pos).await?;
//!     println!("RSSI: {:.2} dBm", rssi);
//!     
//!     Ok(())
//! }
//! ```

pub mod antenna;
pub mod models;
pub mod propagation;
pub mod utils;

pub use antenna::{Antenna, AntennaPattern, PolarizationType};
pub use models::*;
pub use propagation::{PropagationConfig, PropagationModel, RFPropagationEngine, RFResult};

/// Prelude for common RF propagation imports
pub mod prelude {
    pub use crate::antenna::{Antenna, AntennaPattern, PolarizationType};
    pub use crate::models::*;
    pub use crate::propagation::{
        PropagationConfig, PropagationModel, RFPropagationEngine, RFResult,
    };
    pub use crate::utils::*;
}

// Physical constants
pub mod constants {
    /// Speed of light in vacuum (m/s)
    pub const SPEED_OF_LIGHT: f64 = 299_792_458.0;

    /// Boltzmann constant (J/K)
    pub const BOLTZMANN_CONSTANT: f64 = 1.380649e-23;

    /// Permittivity of free space (F/m)
    pub const EPSILON_0: f64 = 8.854187817e-12;

    /// Permeability of free space (H/m)
    pub const MU_0: f64 = 1.25663706212e-6;

    /// Impedance of free space (Ω)
    pub const Z_0: f64 = 376.730313668;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_speed_of_light() {
        assert_eq!(constants::SPEED_OF_LIGHT, 299_792_458.0);
    }
}
