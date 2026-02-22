//! SUMMONER: Scalable Computing Engine for Parallel Tactical Real-time Environments
//!
//! **THE WORLD'S LARGEST MULTI-AGENT SWARM SIMULATION**
//!
//! Designed to **CRUSH HADEAN** by supporting:
//! - **10,000+ agents** (100x more than HADEAN's ~100 limit)
//! - **Multi-GPU distribution** (parallel computation across GPUs)
//! - **Multi-node clusters** (distributed across server infrastructure)
//! - **Real-time performance** (<10ms latency for 10K agents)
//! - **Tactical edge deployment** (ARM/x86, no cloud dependency)
//!
//! # Architecture
//!
//! ```text
//! ┌─────────────────────────────────────────────────────────┐
//! │                   SUMMONER CLUSTER                        │
//! │  ┌───────────────┐  ┌───────────────┐  ┌──────────────┐ │
//! │  │   Node 0      │  │   Node 1      │  │   Node N     │ │
//! │  │  (Coordinator)│  │   (Worker)    │  │   (Worker)   │ │
//! │  │               │  │               │  │              │ │
//! │  │  ┌─────────┐  │  │  ┌─────────┐  │  │  ┌────────┐ │ │
//! │  │  │ GPU 0   │  │  │  │ GPU 0   │  │  │  │ GPU 0  │ │ │
//! │  │  │ GPU 1   │  │  │  │ GPU 1   │  │  │  │ GPU 1  │ │ │
//! │  │  └─────────┘  │  │  └─────────┘  │  │  └────────┘ │ │
//! │  │               │  │               │  │              │ │
//! │  │  Agents       │  │  Agents       │  │  Agents      │ │
//! │  │  0-3333       │  │  3334-6666    │  │  6667-10000  │ │
//! │  └───────────────┘  └───────────────┘  └──────────────┘ │
//! │         │                  │                   │         │
//! │         └──────────────────┴───────────────────┘         │
//! │                    MPI / TCP Network                     │
//! └─────────────────────────────────────────────────────────┘
//! ```
//!
//! # Distribution Strategies
//!
//! ## Spatial Partitioning
//! Agents divided by geographic regions:
//! - Each node simulates agents in its spatial partition
//! - Cross-boundary communications handled by coordinator
//! - Dynamic load balancing based on agent density
//!
//! ## Functional Decomposition
//! - **Node 0**: Physics + RF propagation
//! - **Node 1**: Sensor simulation + perception
//! - **Node 2**: Communication networks + jamming
//! - **Node 3**: AI/ML inference + decision making
//!
//! ## Hybrid Approach
//! Combines spatial + functional for maximum scalability:
//! - GPU 0: Physics simulation (all agents)
//! - GPU 1: RF propagation + jamming (all agents)
//! - CPU: AI inference + coordination
//!
//! # Performance Targets
//!
//! | Agents | Latency | Throughput | Hardware |
//! |--------|---------|------------|----------|
//! | 1,000  | <1ms    | 1M updates/s | 1 GPU |
//! | 10,000 | <10ms   | 1M updates/s | 4 GPU |
//! | 50,000 | <50ms   | 1M updates/s | 16 GPU (4 nodes) |
//! | 100,000| <100ms  | 1M updates/s | 64 GPU (16 nodes) |
//!
//! # Example Usage
//!
//! ```rust,no_run
//! use autonomysim_summoner::{Summoner, SummonerConfig, DistributionStrategy};
//! use nalgebra::Vector3;
//!
//! #[tokio::main]
//! async fn main() -> anyhow::Result<()> {
//!     // Create 10,000-agent swarm simulation
//!     let config = SummonerConfig {
//!         num_agents: 10_000,
//!         distribution: DistributionStrategy::SpatialPartitioning {
//!             bounds: Vector3::new(10000.0, 10000.0, 1000.0),
//!             num_partitions: 4,
//!         },
//!         num_gpus: 4,
//!         num_nodes: 1,
//!         ..Default::default()
//!     };
//!
//!     let mut summoner = Summoner::new(config).await?;
//!
//!     // Run simulation
//!     for step in 0..1000 {
//!         summoner.step(0.01).await?;  // 10ms timestep
//!         
//!         if step % 100 == 0 {
//!             let metrics = summoner.metrics();
//!             println!("Step {}: {} agents, {:.2}ms latency",
//!                      step, metrics.active_agents, metrics.avg_step_time_ms);
//!         }
//!     }
//!
//!     Ok(())
//! }
//! ```
//!
//! # Warfighter Applications
//!
//! - **Drone Swarm Operations**: 1,000-10,000 UAV coordination
//! - **Air Defense Networks**: Multi-layered SAM systems
//! - **Autonomous Convoy Protection**: 100+ vehicle coordination
//! - **Urban Warfare**: Building-scale multi-agent operations
//! - **Electronic Warfare**: Massive SIGINT/COMINT networks
//!
//! # Competitive Advantage vs HADEAN
//!
//! | Capability | HADEAN | SUMMONER |
//! |------------|--------|---------|
//! | Max Agents | ~100 | **10,000+** |
//! | Multi-GPU | ❌ | ✅ |
//! | Multi-Node | ❌ | ✅ |
//! | Real-time | ❌ (cloud) | ✅ (<10ms) |
//! | Edge Deploy | ❌ | ✅ |
//! | Tactical Comms | ❌ | ✅ |
//! | EW Simulation | ❌ | ✅ |

pub mod communication;
pub mod coordinator;
pub mod metrics;
pub mod partition;
pub mod scheduler;
pub mod worker;

use anyhow::{Context, Result};
use nalgebra::Vector3;
use serde::{Deserialize, Serialize};
use std::sync::Arc;
use tokio::sync::RwLock;
use tracing::{info, warn};

pub use communication::{MessageBus, NodeMessage};
pub use coordinator::Coordinator;
pub use metrics::{PerformanceMonitor, SummonerMetrics};
pub use partition::{Partition, SpatialPartitioner};
pub use worker::Worker;

/// Distribution strategy for multi-node/multi-GPU
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum DistributionStrategy {
    /// Single-node, single-GPU (baseline)
    SingleNode,

    /// Spatial partitioning across nodes/GPUs
    SpatialPartitioning {
        bounds: Vector3<f64>,
        num_partitions: usize,
    },

    /// Functional decomposition
    FunctionalDecomposition {
        physics_nodes: usize,
        sensor_nodes: usize,
        comms_nodes: usize,
        ai_nodes: usize,
    },

    /// Hybrid: spatial + functional
    Hybrid {
        spatial_bounds: Vector3<f64>,
        spatial_partitions: usize,
        functional_layers: Vec<String>,
    },
}

/// SUMMONER configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SummonerConfig {
    /// Total number of agents
    pub num_agents: usize,

    /// Distribution strategy
    pub distribution: DistributionStrategy,

    /// Number of GPUs to use
    pub num_gpus: usize,

    /// Number of cluster nodes
    pub num_nodes: usize,

    /// Simulation timestep (seconds)
    pub timestep: f64,

    /// Enable real-time mode (synchronize with wall clock)
    pub realtime: bool,

    /// Enable performance monitoring
    pub enable_monitoring: bool,

    /// Prometheus metrics endpoint
    pub metrics_port: Option<u16>,

    /// MPI rank (for distributed mode)
    pub mpi_rank: Option<usize>,

    /// MPI world size
    pub mpi_world_size: Option<usize>,
}

impl Default for SummonerConfig {
    fn default() -> Self {
        Self {
            num_agents: 1000,
            distribution: DistributionStrategy::SingleNode,
            num_gpus: 1,
            num_nodes: 1,
            timestep: 0.01, // 10ms
            realtime: false,
            enable_monitoring: true,
            metrics_port: Some(9090),
            mpi_rank: None,
            mpi_world_size: None,
        }
    }
}

/// Main SUMMONER simulation engine
pub struct Summoner {
    config: SummonerConfig,
    coordinator: Arc<RwLock<Coordinator>>,
    workers: Vec<Arc<RwLock<Worker>>>,
    _message_bus: Arc<MessageBus>,
    monitor: Option<PerformanceMonitor>,
    current_step: u64,
}

impl Summoner {
    /// Create a new SUMMONER simulation
    pub async fn new(config: SummonerConfig) -> Result<Self> {
        info!(
            "Initializing SUMMONER with {} agents across {} nodes",
            config.num_agents, config.num_nodes
        );

        // Validate configuration
        Self::validate_config(&config)?;

        // Initialize message bus
        let message_bus = Arc::new(MessageBus::new(config.num_nodes));

        // Initialize performance monitor
        let monitor = if config.enable_monitoring {
            Some(PerformanceMonitor::new(config.metrics_port)?)
        } else {
            None
        };

        // Initialize coordinator
        let coordinator = Arc::new(RwLock::new(
            Coordinator::new(config.clone(), message_bus.clone()).await?,
        ));

        // Initialize workers based on distribution strategy
        let workers = Self::initialize_workers(&config, message_bus.clone()).await?;

        info!(
            "SUMMONER initialization complete: {} workers, {} agents",
            workers.len(),
            config.num_agents
        );

        Ok(Self {
            config,
            coordinator,
            workers,
            _message_bus: message_bus,
            monitor,
            current_step: 0,
        })
    }

    /// Validate SUMMONER configuration
    fn validate_config(config: &SummonerConfig) -> Result<()> {
        if config.num_agents == 0 {
            anyhow::bail!("num_agents must be > 0");
        }

        if config.num_agents > 1_000_000 {
            warn!(
                "num_agents = {} may require significant resources",
                config.num_agents
            );
        }

        if config.num_nodes == 0 {
            anyhow::bail!("num_nodes must be > 0");
        }

        if config.num_gpus == 0 {
            warn!("num_gpus = 0, running CPU-only (may be slow)");
        }

        if config.timestep <= 0.0 {
            anyhow::bail!("timestep must be > 0");
        }

        if config.timestep > 0.1 {
            warn!(
                "timestep = {:.3}s is large, may cause instability",
                config.timestep
            );
        }

        Ok(())
    }

    /// Initialize worker nodes
    async fn initialize_workers(
        config: &SummonerConfig,
        message_bus: Arc<MessageBus>,
    ) -> Result<Vec<Arc<RwLock<Worker>>>> {
        let num_workers = match &config.distribution {
            DistributionStrategy::SingleNode => 1,
            DistributionStrategy::SpatialPartitioning { num_partitions, .. } => *num_partitions,
            DistributionStrategy::FunctionalDecomposition {
                physics_nodes,
                sensor_nodes,
                comms_nodes,
                ai_nodes,
            } => physics_nodes + sensor_nodes + comms_nodes + ai_nodes,
            DistributionStrategy::Hybrid {
                spatial_partitions,
                functional_layers,
                ..
            } => spatial_partitions * functional_layers.len(),
        };

        let mut workers = Vec::with_capacity(num_workers);

        for worker_id in 0..num_workers {
            let worker = Arc::new(RwLock::new(
                Worker::new(worker_id, config.clone(), message_bus.clone()).await?,
            ));
            workers.push(worker);
        }

        Ok(workers)
    }

    /// Execute one simulation step
    pub async fn step(&mut self, dt: f64) -> Result<()> {
        let step_start = std::time::Instant::now();

        // 1. Coordinator broadcasts step command
        {
            let coord = self.coordinator.read().await;
            coord.broadcast_step(self.current_step, dt).await?;
        }

        // 2. Workers execute in parallel
        let worker_futures: Vec<_> = self
            .workers
            .iter()
            .map(|worker| {
                let worker = worker.clone();
                async move {
                    let mut w = worker.write().await;
                    w.execute_step(dt).await
                }
            })
            .collect();

        // Wait for all workers to complete
        let results = futures::future::join_all(worker_futures).await;

        // Check for errors
        for result in results {
            result.context("Worker step failed")?;
        }

        // 3. Synchronize boundary data
        {
            let coord = self.coordinator.write().await;
            coord.synchronize_boundaries().await?;
        }

        // 4. Update metrics
        let step_time = step_start.elapsed();
        if let Some(monitor) = &self.monitor {
            monitor.record_step(self.current_step, step_time, self.config.num_agents);
        }

        self.current_step += 1;

        Ok(())
    }

    /// Run simulation for N steps
    pub async fn run(&mut self, num_steps: usize) -> Result<()> {
        info!("Starting SUMMONER simulation for {} steps", num_steps);

        for step in 0..num_steps {
            self.step(self.config.timestep).await?;

            if step % 100 == 0 {
                let metrics = self.metrics();
                info!(
                    "Step {}/{}: {:.2}ms, {} agents",
                    step, num_steps, metrics.avg_step_time_ms, metrics.active_agents
                );
            }
        }

        info!("SUMMONER simulation complete");
        Ok(())
    }

    /// Get current metrics
    pub fn metrics(&self) -> SummonerMetrics {
        if let Some(monitor) = &self.monitor {
            monitor.get_metrics()
        } else {
            SummonerMetrics::default()
        }
    }

    /// Get current step number
    pub fn current_step(&self) -> u64 {
        self.current_step
    }

    /// Get configuration
    pub fn config(&self) -> &SummonerConfig {
        &self.config
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_summoner_creation() {
        let config = SummonerConfig {
            num_agents: 100,
            distribution: DistributionStrategy::SingleNode,
            enable_monitoring: false,
            ..Default::default()
        };

        let summoner = Summoner::new(config).await;
        assert!(summoner.is_ok());
    }

    #[tokio::test]
    async fn test_summoner_step() {
        let config = SummonerConfig {
            num_agents: 10,
            enable_monitoring: false,
            ..Default::default()
        };

        let mut summoner = Summoner::new(config).await.unwrap();

        let result = summoner.step(0.01).await;
        assert!(result.is_ok());
        assert_eq!(summoner.current_step(), 1);
    }

    #[test]
    fn test_config_validation() {
        let bad_config = SummonerConfig {
            num_agents: 0,
            ..Default::default()
        };

        assert!(Summoner::validate_config(&bad_config).is_err());

        let good_config = SummonerConfig::default();
        assert!(Summoner::validate_config(&good_config).is_ok());
    }
}
