//! Backend implementations for different simulation engines
//!
//! This crate provides implementations of the `SimulationBackend` trait
//! for various simulation engines.
//!
//! # Supported Backends
//!
//! - **Unreal Engine 5** (feature: `unreal`) - ✅ Structure Implemented
//! - **NVIDIA Isaac Lab** (feature: `isaac`) - ✅ Implemented with Python bridge
//! - **MuJoCo** (feature: `mujoco`) - ✅ Implemented with C FFI
//! - **NVIDIA Warp** (feature: `warp`) - ✅ Implemented with GPU compute
//!
//! # Usage
//!
//! ```toml
//! [dependencies]
//! autonomysim-backends = { version = "0.1", features = ["unreal"] }
//! ```

#[cfg(feature = "unreal")]
pub mod unreal;

#[cfg(feature = "isaac")]
pub mod isaac;

#[cfg(feature = "mujoco")]
pub mod mujoco;

#[cfg(feature = "warp")]
pub mod warp;

// Re-export backends
#[cfg(feature = "unreal")]
pub use unreal::UnrealEngine5Backend;

#[cfg(feature = "isaac")]
pub use isaac::IsaacLabBackend;

#[cfg(feature = "mujoco")]
pub use mujoco::MuJoCoBackend;

#[cfg(feature = "warp")]
pub use warp::WarpBackend;
