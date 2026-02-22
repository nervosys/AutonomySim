//! Communication infrastructure for distributed simulation

use anyhow::Result;
use serde::{Deserialize, Serialize};
use std::sync::Arc;
use tokio::sync::{mpsc, RwLock};
use tracing::debug;

/// Messages exchanged between nodes
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum NodeMessage {
    /// Command to execute simulation step
    StepCommand { step: u64, dt: f64 },

    /// Synchronize boundary data
    SyncBoundaries,

    /// Agent state update for boundary sync
    AgentState {
        agent_id: usize,
        position: [f64; 3],
        velocity: [f64; 3],
    },

    /// Heartbeat from worker
    Heartbeat { worker_id: usize },

    /// Worker completed step
    StepComplete { worker_id: usize, step: u64 },
}

/// Message bus for inter-node communication
pub struct MessageBus {
    _num_nodes: usize,
    channels: Arc<RwLock<Vec<mpsc::UnboundedSender<NodeMessage>>>>,
}

impl MessageBus {
    /// Create new message bus
    pub fn new(num_nodes: usize) -> Self {
        Self {
            _num_nodes: num_nodes,
            channels: Arc::new(RwLock::new(Vec::new())),
        }
    }

    /// Register a channel for a node
    pub async fn register_channel(&self, sender: mpsc::UnboundedSender<NodeMessage>) {
        let mut channels = self.channels.write().await;
        channels.push(sender);
    }

    /// Broadcast message to all nodes
    pub async fn broadcast(&self, message: NodeMessage) -> Result<()> {
        debug!("Broadcasting message: {:?}", message);

        let channels = self.channels.read().await;
        for sender in channels.iter() {
            sender.send(message.clone())?;
        }

        Ok(())
    }

    /// Send message to specific node
    pub async fn send_to(&self, node_id: usize, message: NodeMessage) -> Result<()> {
        let channels = self.channels.read().await;
        if let Some(sender) = channels.get(node_id) {
            sender.send(message)?;
            Ok(())
        } else {
            anyhow::bail!("Node {} not found", node_id);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_message_bus() {
        let bus = MessageBus::new(2);

        let (tx, mut rx) = mpsc::unbounded_channel();
        bus.register_channel(tx).await;

        let message = NodeMessage::Heartbeat { worker_id: 0 };
        bus.broadcast(message.clone()).await.unwrap();

        let received = rx.recv().await;
        assert!(received.is_some());
    }
}
