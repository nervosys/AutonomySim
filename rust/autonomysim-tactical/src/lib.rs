//! Tactical RF Warfare Module
//!
//! This module provides military-grade electronic warfare capabilities including:
//! - Jamming models (barrage, follower, swept)
//! - Network resilience (partition detection, mesh healing)
//! - MANET protocols (OLSR, AODV simulation)
//! - Spectrum management (dynamic frequency allocation)
//! - Link quality metrics (SNR, BER, packet loss)
//!
//! # Example
//!
//! ```rust
//! use autonomysim_tactical::jamming::{JammingModel, JammingType, JammingConfig};
//! use nalgebra::Vector3;
//!
//! // Create a barrage jammer at origin with 100W power
//! let config = JammingConfig {
//!     jammer_position: Vector3::new(0.0, 0.0, 100.0),
//!     jammer_power_dbm: 50.0,  // 100W = 50 dBm
//!     jamming_type: JammingType::Barrage,
//!     center_frequency_hz: 2.4e9,
//!     bandwidth_hz: 20e6,
//!     ..Default::default()
//! };
//!
//! let jammer = JammingModel::new(config);
//!
//! // Calculate jamming effectiveness at a target position
//! let target = Vector3::new(1000.0, 0.0, 50.0);
//! let signal_dbm = -70.0;  // Desired signal strength
//! let jamming_dbm = jammer.compute_jamming_power(target);
//! let jsr = jammer.compute_jamming_to_signal_ratio(signal_dbm, jamming_dbm);
//!
//! println!("J/S Ratio: {:.2} dB (>10 dB = effective jamming)", jsr);
//! ```
//!
//! # Tactical Applications
//!
//! - **Drone Swarm Communications**: Model communication link quality under jamming
//! - **Electronic Warfare Training**: Simulate adversary EW tactics
//! - **Spectrum Planning**: Optimize frequency allocation to avoid jamming
//! - **Network Resilience**: Test mesh network recovery under denial
//! - **Counter-EW Development**: Train AI to adapt to jamming

pub mod jamming;
pub mod metrics;
pub mod network;
pub mod spectrum;

// Re-export commonly used types
pub use jamming::{jsr_thresholds, JammingConfig, JammingModel, JammingType};
pub use metrics::{
    db_to_linear, dbm_to_watts, linear_to_db, watts_to_dbm, BERCalculator, LinkBudget,
    ModulationScheme, PERCalculator, SignalMetrics,
};
pub use network::{AgentId, LinkQuality, LinkState, NetworkTopology, PartitionDetector};
pub use spectrum::{Channel, FrequencyAllocation, SpectrumManager};
