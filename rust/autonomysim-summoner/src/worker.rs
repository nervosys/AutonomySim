//! Worker: Executes simulation on subset of agents

use anyhow::Result;
use std::sync::Arc;
use tracing::{debug, info};

use crate::{MessageBus, SummonerConfig};

/// Worker node that simulates a subset of agents
pub struct Worker {
    worker_id: usize,
    config: SummonerConfig,
    message_bus: Arc<MessageBus>,
    agent_ids: Vec<usize>,
    current_step: u64,
}

impl Worker {
    /// Create new worker
    pub async fn new(
        worker_id: usize,
        config: SummonerConfig,
        message_bus: Arc<MessageBus>,
    ) -> Result<Self> {
        info!("Initializing worker {}", worker_id);

        // Assign agents to this worker based on distribution strategy
        let agent_ids = Self::assign_agents(worker_id, &config);

        info!("Worker {} assigned {} agents", worker_id, agent_ids.len());

        Ok(Self {
            worker_id,
            config,
            message_bus,
            agent_ids,
            current_step: 0,
        })
    }

    /// Assign agents to this worker
    fn assign_agents(worker_id: usize, config: &SummonerConfig) -> Vec<usize> {
        let num_workers = match &config.distribution {
            crate::DistributionStrategy::SingleNode => 1,
            crate::DistributionStrategy::SpatialPartitioning { num_partitions, .. } => {
                *num_partitions
            }
            crate::DistributionStrategy::FunctionalDecomposition {
                physics_nodes,
                sensor_nodes,
                comms_nodes,
                ai_nodes,
            } => physics_nodes + sensor_nodes + comms_nodes + ai_nodes,
            crate::DistributionStrategy::Hybrid {
                spatial_partitions,
                functional_layers,
                ..
            } => spatial_partitions * functional_layers.len(),
        };

        // Simple round-robin assignment
        let agents_per_worker = config.num_agents / num_workers;
        let start = worker_id * agents_per_worker;
        let end = if worker_id == num_workers - 1 {
            config.num_agents // Last worker takes remainder
        } else {
            start + agents_per_worker
        };

        (start..end).collect()
    }

    /// Execute one simulation step for this worker's agents
    pub async fn execute_step(&mut self, dt: f64) -> Result<()> {
        debug!(
            "Worker {} executing step {} with dt={}",
            self.worker_id, self.current_step, dt
        );

        // Simulate physics for assigned agents
        self.simulate_physics(dt).await?;

        // Simulate sensors
        self.simulate_sensors(dt).await?;

        // Simulate communications
        self.simulate_communications(dt).await?;

        // Update AI/control
        self.update_control(dt).await?;

        self.current_step += 1;

        Ok(())
    }

    /// Simulate physics for agents
    async fn simulate_physics(&self, _dt: f64) -> Result<()> {
        // TODO: Integrate with autonomysim-backends for actual physics
        // For now, placeholder
        debug!(
            "Worker {} simulating physics for {} agents",
            self.worker_id,
            self.agent_ids.len()
        );
        Ok(())
    }

    /// Simulate sensors
    async fn simulate_sensors(&self, _dt: f64) -> Result<()> {
        // TODO: Simulate IMU, GPS, camera, lidar, etc.
        debug!("Worker {} simulating sensors", self.worker_id);
        Ok(())
    }

    /// Simulate communications and jamming
    async fn simulate_communications(&self, _dt: f64) -> Result<()> {
        // TODO: Integrate autonomysim-tactical for RF/jamming
        debug!("Worker {} simulating communications", self.worker_id);
        Ok(())
    }

    /// Update control and AI
    async fn update_control(&self, _dt: f64) -> Result<()> {
        // TODO: Run AI inference, control laws
        debug!("Worker {} updating control", self.worker_id);
        Ok(())
    }

    /// Get worker ID
    pub fn worker_id(&self) -> usize {
        self.worker_id
    }

    /// Get number of agents
    pub fn num_agents(&self) -> usize {
        self.agent_ids.len()
    }
}
