//! Coordinator: Central orchestrator for distributed SUMMONER simulation

use anyhow::Result;
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;
use tracing::{debug, info};

use crate::{MessageBus, NodeMessage, SummonerConfig};

/// Coordinator state
pub struct Coordinator {
    _config: SummonerConfig,
    _message_bus: Arc<MessageBus>,
    worker_states: Arc<RwLock<HashMap<usize, WorkerState>>>,
    current_step: u64,
}

/// Worker node state
#[derive(Debug, Clone)]
pub struct WorkerState {
    pub worker_id: usize,
    pub num_agents: usize,
    pub last_heartbeat: std::time::Instant,
    pub is_healthy: bool,
}

impl Coordinator {
    /// Create new coordinator
    pub async fn new(config: SummonerConfig, message_bus: Arc<MessageBus>) -> Result<Self> {
        info!("Initializing coordinator for {} nodes", config.num_nodes);

        Ok(Self {
            _config: config,
            _message_bus: message_bus,
            worker_states: Arc::new(RwLock::new(HashMap::new())),
            current_step: 0,
        })
    }

    /// Register a worker node
    pub async fn register_worker(&self, worker_id: usize, num_agents: usize) -> Result<()> {
        let mut states = self.worker_states.write().await;
        states.insert(
            worker_id,
            WorkerState {
                worker_id,
                num_agents,
                last_heartbeat: std::time::Instant::now(),
                is_healthy: true,
            },
        );

        info!("Registered worker {} with {} agents", worker_id, num_agents);
        Ok(())
    }

    /// Broadcast step command to all workers
    pub async fn broadcast_step(&self, step: u64, dt: f64) -> Result<()> {
        debug!("Broadcasting step {} to all workers", step);

        let message = NodeMessage::StepCommand { step, dt };
        self._message_bus.broadcast(message).await?;

        Ok(())
    }

    /// Synchronize boundary data between workers
    pub async fn synchronize_boundaries(&self) -> Result<()> {
        debug!("Synchronizing boundaries for step {}", self.current_step);

        // In spatial partitioning, agents near partition boundaries
        // need to exchange state with neighboring partitions

        let states = self.worker_states.read().await;

        for (worker_id, state) in states.iter() {
            if !state.is_healthy {
                anyhow::bail!("Worker {} unhealthy, cannot synchronize", worker_id);
            }
        }

        // Send boundary sync command
        let message = NodeMessage::SyncBoundaries;
        self._message_bus.broadcast(message).await?;

        Ok(())
    }

    /// Check health of all workers
    pub async fn check_worker_health(&self) -> Result<()> {
        let mut states = self.worker_states.write().await;
        let timeout = std::time::Duration::from_secs(5);

        for (worker_id, state) in states.iter_mut() {
            if state.last_heartbeat.elapsed() > timeout {
                state.is_healthy = false;
                anyhow::bail!("Worker {} timeout", worker_id);
            }
        }

        Ok(())
    }

    /// Get total agent count across all workers
    pub async fn total_agents(&self) -> usize {
        let states = self.worker_states.read().await;
        states.values().map(|s| s.num_agents).sum()
    }
}
