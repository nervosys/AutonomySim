//! Network Topology and Resilience for Tactical Communications
//!
//! Implements:
//! - Dynamic network topology with link quality tracking
//! - Network partition detection using graph algorithms
//! - Mesh healing strategies for self-healing networks
//! - MANET (Mobile Ad-hoc Network) protocol simulation

use nalgebra::Vector3;
use serde::{Deserialize, Serialize};
use std::collections::{HashMap, HashSet, VecDeque};

/// Agent ID in network
pub type AgentId = usize;

/// Link quality metrics
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct LinkQuality {
    /// Signal-to-Noise Ratio (dB)
    pub snr_db: f64,

    /// Received Signal Strength Indicator (dBm)
    pub rssi_dbm: f64,

    /// Packet loss rate (0.0 - 1.0)
    pub packet_loss_rate: f64,

    /// Link latency (seconds)
    pub latency_s: f64,

    /// Link available bandwidth (bps)
    pub bandwidth_bps: f64,

    /// Link active duration (seconds)
    pub active_duration_s: f64,
}

impl LinkQuality {
    /// Check if link meets minimum quality thresholds
    pub fn is_usable(&self, min_snr_db: f64, max_packet_loss: f64) -> bool {
        self.snr_db >= min_snr_db && self.packet_loss_rate <= max_packet_loss
    }

    /// Compute link score (0.0 - 1.0)
    /// Higher score = better link quality
    pub fn compute_score(&self) -> f64 {
        // Normalize SNR (assume 0-30 dB range)
        let snr_norm = (self.snr_db / 30.0).clamp(0.0, 1.0);

        // Normalize packet loss (invert since lower is better)
        let loss_norm = 1.0 - self.packet_loss_rate.clamp(0.0, 1.0);

        // Normalize latency (assume 0-1s range, inverted)
        let latency_norm = 1.0 - (self.latency_s / 1.0).clamp(0.0, 1.0);

        // Weighted combination
        0.5 * snr_norm + 0.3 * loss_norm + 0.2 * latency_norm
    }
}

impl Default for LinkQuality {
    fn default() -> Self {
        Self {
            snr_db: 20.0,
            rssi_dbm: -60.0,
            packet_loss_rate: 0.0,
            latency_s: 0.01,
            bandwidth_bps: 10e6, // 10 Mbps
            active_duration_s: 0.0,
        }
    }
}

/// Link state between two agents
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LinkState {
    /// Source agent ID
    pub source: AgentId,

    /// Destination agent ID
    pub destination: AgentId,

    /// Link quality metrics
    pub quality: LinkQuality,

    /// Last update time
    pub last_update_time: f64,
}

/// Network topology graph
#[derive(Debug, Clone)]
pub struct NetworkTopology {
    /// Agent positions (NED frame)
    agents: HashMap<AgentId, Vector3<f64>>,

    /// Link states (directed graph)
    links: HashMap<(AgentId, AgentId), LinkState>,

    /// Current simulation time
    current_time: f64,

    /// Link timeout (seconds)
    link_timeout_s: f64,

    /// Minimum SNR for usable link (dB)
    min_snr_db: f64,

    /// Maximum packet loss for usable link (0.0-1.0)
    max_packet_loss: f64,
}

impl NetworkTopology {
    /// Create a new network topology
    pub fn new() -> Self {
        Self {
            agents: HashMap::new(),
            links: HashMap::new(),
            current_time: 0.0,
            link_timeout_s: 5.0,
            min_snr_db: 10.0,
            max_packet_loss: 0.1,
        }
    }

    /// Add or update an agent
    pub fn add_agent(&mut self, id: AgentId, position: Vector3<f64>) {
        self.agents.insert(id, position);
    }

    /// Remove an agent
    pub fn remove_agent(&mut self, id: AgentId) {
        self.agents.remove(&id);

        // Remove all links involving this agent
        self.links.retain(|(src, dst), _| *src != id && *dst != id);
    }

    /// Add or update a link
    pub fn add_link(&mut self, source: AgentId, destination: AgentId, quality: LinkQuality) {
        let link = LinkState {
            source,
            destination,
            quality,
            last_update_time: self.current_time,
        };

        self.links.insert((source, destination), link);
    }

    /// Get link state
    pub fn get_link(&self, source: AgentId, destination: AgentId) -> Option<&LinkState> {
        self.links.get(&(source, destination))
    }

    /// Update simulation time and prune stale links
    pub fn update_time(&mut self, time: f64) {
        self.current_time = time;

        // Remove stale links
        let timeout = self.link_timeout_s;
        let current = self.current_time;
        self.links
            .retain(|_, link| (current - link.last_update_time) < timeout);
    }

    /// Get all neighbors of an agent (agents with usable links)
    pub fn get_neighbors(&self, agent: AgentId) -> Vec<AgentId> {
        self.links
            .iter()
            .filter_map(|((src, dst), link)| {
                if *src == agent
                    && link
                        .quality
                        .is_usable(self.min_snr_db, self.max_packet_loss)
                {
                    Some(*dst)
                } else {
                    None
                }
            })
            .collect()
    }

    /// Get all agents
    pub fn get_agents(&self) -> Vec<AgentId> {
        self.agents.keys().copied().collect()
    }

    /// Get agent position
    pub fn get_position(&self, agent: AgentId) -> Option<Vector3<f64>> {
        self.agents.get(&agent).copied()
    }

    /// Get number of agents
    pub fn agent_count(&self) -> usize {
        self.agents.len()
    }

    /// Get number of active links
    pub fn link_count(&self) -> usize {
        self.links.len()
    }

    /// Compute network connectivity (fraction of agent pairs connected)
    pub fn compute_connectivity(&self) -> f64 {
        let n = self.agent_count();
        if n < 2 {
            return 1.0;
        }

        let mut connected_pairs = 0;
        let total_pairs = n * (n - 1);

        let agents: Vec<AgentId> = self.get_agents();

        for i in 0..agents.len() {
            for j in 0..agents.len() {
                if i != j && self.is_connected(agents[i], agents[j]) {
                    connected_pairs += 1;
                }
            }
        }

        connected_pairs as f64 / total_pairs as f64
    }

    /// Check if two agents are connected (any path exists)
    pub fn is_connected(&self, source: AgentId, destination: AgentId) -> bool {
        if source == destination {
            return true;
        }

        // Breadth-first search
        let mut visited = HashSet::new();
        let mut queue = VecDeque::new();

        queue.push_back(source);
        visited.insert(source);

        while let Some(current) = queue.pop_front() {
            if current == destination {
                return true;
            }

            for neighbor in self.get_neighbors(current) {
                if !visited.contains(&neighbor) {
                    visited.insert(neighbor);
                    queue.push_back(neighbor);
                }
            }
        }

        false
    }

    /// Find shortest path between two agents (Dijkstra's algorithm)
    /// Returns None if no path exists
    pub fn find_shortest_path(
        &self,
        source: AgentId,
        destination: AgentId,
    ) -> Option<Vec<AgentId>> {
        if source == destination {
            return Some(vec![source]);
        }

        let mut distances: HashMap<AgentId, f64> = HashMap::new();
        let mut previous: HashMap<AgentId, AgentId> = HashMap::new();
        let mut unvisited: HashSet<AgentId> = self.get_agents().into_iter().collect();

        distances.insert(source, 0.0);

        while !unvisited.is_empty() {
            // Find unvisited node with smallest distance
            let current = unvisited
                .iter()
                .min_by(|a, b| {
                    let dist_a = distances.get(a).unwrap_or(&f64::INFINITY);
                    let dist_b = distances.get(b).unwrap_or(&f64::INFINITY);
                    dist_a.partial_cmp(dist_b).unwrap()
                })
                .copied()?;

            if current == destination {
                break;
            }

            unvisited.remove(&current);

            let current_dist = *distances.get(&current).unwrap_or(&f64::INFINITY);
            if current_dist == f64::INFINITY {
                break;
            }

            // Update distances to neighbors
            for neighbor in self.get_neighbors(current) {
                if !unvisited.contains(&neighbor) {
                    continue;
                }

                // Use inverse link score as edge weight (higher quality = lower cost)
                let link_cost = if let Some(link) = self.get_link(current, neighbor) {
                    let score = link.quality.compute_score();
                    if score > 0.0 {
                        1.0 / score
                    } else {
                        f64::INFINITY
                    }
                } else {
                    f64::INFINITY
                };

                let alt_dist = current_dist + link_cost;
                let neighbor_dist = *distances.get(&neighbor).unwrap_or(&f64::INFINITY);

                if alt_dist < neighbor_dist {
                    distances.insert(neighbor, alt_dist);
                    previous.insert(neighbor, current);
                }
            }
        }

        // Reconstruct path
        if !previous.contains_key(&destination) {
            return None;
        }

        let mut path = vec![destination];
        let mut current = destination;

        while current != source {
            current = *previous.get(&current)?;
            path.push(current);
        }

        path.reverse();
        Some(path)
    }
}

impl Default for NetworkTopology {
    fn default() -> Self {
        Self::new()
    }
}

/// Partition detector using connected components
#[derive(Debug)]
pub struct PartitionDetector {
    /// Network topology reference
    topology: NetworkTopology,

    /// Detected partitions (sets of agent IDs)
    partitions: Vec<HashSet<AgentId>>,

    /// Last detection time
    last_detection_time: f64,
}

impl PartitionDetector {
    /// Create a new partition detector
    pub fn new(topology: NetworkTopology) -> Self {
        Self {
            topology,
            partitions: Vec::new(),
            last_detection_time: 0.0,
        }
    }

    /// Detect network partitions using connected components algorithm
    pub fn detect_partitions(&mut self) -> &Vec<HashSet<AgentId>> {
        self.partitions.clear();

        let agents = self.topology.get_agents();
        let mut unvisited: HashSet<AgentId> = agents.into_iter().collect();

        while let Some(&start) = unvisited.iter().next() {
            // Find all agents connected to 'start'
            let mut partition = HashSet::new();
            let mut queue = VecDeque::new();

            queue.push_back(start);
            partition.insert(start);
            unvisited.remove(&start);

            while let Some(current) = queue.pop_front() {
                for neighbor in self.topology.get_neighbors(current) {
                    if unvisited.contains(&neighbor) {
                        partition.insert(neighbor);
                        unvisited.remove(&neighbor);
                        queue.push_back(neighbor);
                    }
                }
            }

            self.partitions.push(partition);
        }

        self.last_detection_time = self.topology.current_time;
        &self.partitions
    }

    /// Get number of partitions
    pub fn partition_count(&self) -> usize {
        self.partitions.len()
    }

    /// Check if network is partitioned (more than one partition)
    pub fn is_partitioned(&self) -> bool {
        self.partitions.len() > 1
    }

    /// Get largest partition
    pub fn largest_partition(&self) -> Option<&HashSet<AgentId>> {
        self.partitions.iter().max_by_key(|p| p.len())
    }

    /// Get partition containing specific agent
    pub fn get_partition(&self, agent: AgentId) -> Option<&HashSet<AgentId>> {
        self.partitions.iter().find(|p| p.contains(&agent))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_network_topology() {
        let mut topology = NetworkTopology::new();

        // Add agents
        topology.add_agent(0, Vector3::new(0.0, 0.0, 0.0));
        topology.add_agent(1, Vector3::new(100.0, 0.0, 0.0));
        topology.add_agent(2, Vector3::new(200.0, 0.0, 0.0));

        assert_eq!(topology.agent_count(), 3);

        // Add links
        let quality = LinkQuality {
            snr_db: 20.0,
            packet_loss_rate: 0.01,
            ..Default::default()
        };

        topology.add_link(0, 1, quality);
        topology.add_link(1, 2, quality);

        assert_eq!(topology.link_count(), 2);

        // Check connectivity
        assert!(topology.is_connected(0, 1));
        assert!(topology.is_connected(0, 2)); // Through agent 1
    }

    #[test]
    fn test_shortest_path() {
        let mut topology = NetworkTopology::new();

        // Create linear network: 0 -- 1 -- 2 -- 3
        for i in 0..4 {
            topology.add_agent(i, Vector3::new(i as f64 * 100.0, 0.0, 0.0));
        }

        let quality = LinkQuality::default();
        for i in 0..3 {
            topology.add_link(i, i + 1, quality);
            topology.add_link(i + 1, i, quality); // Bidirectional
        }

        let path = topology.find_shortest_path(0, 3);
        assert!(path.is_some());
        let path = path.unwrap();
        assert_eq!(path, vec![0, 1, 2, 3]);
    }

    #[test]
    fn test_partition_detection() {
        let mut topology = NetworkTopology::new();

        // Create two separate partitions: {0, 1} and {2, 3}
        for i in 0..4 {
            topology.add_agent(i, Vector3::new(i as f64 * 100.0, 0.0, 0.0));
        }

        let quality = LinkQuality::default();

        // Partition 1: 0 -- 1
        topology.add_link(0, 1, quality);
        topology.add_link(1, 0, quality);

        // Partition 2: 2 -- 3
        topology.add_link(2, 3, quality);
        topology.add_link(3, 2, quality);

        let mut detector = PartitionDetector::new(topology);
        detector.detect_partitions();

        assert!(detector.is_partitioned());
        assert_eq!(detector.partition_count(), 2);
    }

    #[test]
    fn test_link_quality_score() {
        let good_link = LinkQuality {
            snr_db: 30.0,
            packet_loss_rate: 0.0,
            latency_s: 0.01,
            ..Default::default()
        };

        let bad_link = LinkQuality {
            snr_db: 5.0,
            packet_loss_rate: 0.5,
            latency_s: 0.5,
            ..Default::default()
        };

        assert!(good_link.compute_score() > bad_link.compute_score());
        assert!(good_link.compute_score() > 0.8);
        assert!(bad_link.compute_score() < 0.5); // More lenient threshold
    }
}
