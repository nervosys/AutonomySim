//! Spectrum Management for Tactical Communications
//!
//! Implements:
//! - Dynamic frequency allocation
//! - Co-channel interference calculation
//! - Frequency hopping pattern generation
//! - Spectrum occupancy tracking

use serde::{Deserialize, Serialize};
use std::collections::{HashMap, HashSet};

use crate::network::AgentId;

/// Frequency channel
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Channel {
    /// Channel ID
    pub id: usize,

    /// Center frequency (Hz)
    pub frequency_hz: u64,

    /// Channel bandwidth (Hz)
    pub bandwidth_hz: u64,
}

impl Channel {
    /// Create a new channel
    pub fn new(id: usize, frequency_hz: u64, bandwidth_hz: u64) -> Self {
        Self {
            id,
            frequency_hz,
            bandwidth_hz,
        }
    }

    /// Check if this channel overlaps with another
    pub fn overlaps(&self, other: &Channel) -> bool {
        let self_min = self.frequency_hz - self.bandwidth_hz / 2;
        let self_max = self.frequency_hz + self.bandwidth_hz / 2;
        let other_min = other.frequency_hz - other.bandwidth_hz / 2;
        let other_max = other.frequency_hz + other.bandwidth_hz / 2;

        !(self_max < other_min || other_max < self_min)
    }

    /// Compute frequency separation (Hz)
    pub fn frequency_separation(&self, other: &Channel) -> u64 {
        if self.frequency_hz > other.frequency_hz {
            self.frequency_hz - other.frequency_hz
        } else {
            other.frequency_hz - self.frequency_hz
        }
    }
}

/// Frequency allocation for an agent
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FrequencyAllocation {
    /// Agent ID
    pub agent_id: AgentId,

    /// Allocated channel
    pub channel: Channel,

    /// Allocation time
    pub allocation_time: f64,

    /// Transmit power (dBm)
    pub tx_power_dbm: f64,
}

/// Spectrum manager
pub struct SpectrumManager {
    /// Available channels
    channels: Vec<Channel>,

    /// Current allocations (agent -> channel)
    allocations: HashMap<AgentId, FrequencyAllocation>,

    /// Channel usage tracking (channel -> set of agents)
    channel_usage: HashMap<usize, HashSet<AgentId>>,

    /// Current simulation time
    current_time: f64,

    /// Minimum frequency separation for interference-free operation (Hz)
    min_separation_hz: u64,
}

impl SpectrumManager {
    /// Create a new spectrum manager
    pub fn new(channels: Vec<Channel>) -> Self {
        Self {
            channels,
            allocations: HashMap::new(),
            channel_usage: HashMap::new(),
            current_time: 0.0,
            min_separation_hz: 5_000_000, // 5 MHz default
        }
    }

    /// Create a spectrum manager with equally spaced channels
    pub fn with_uniform_channels(
        start_frequency_hz: u64,
        end_frequency_hz: u64,
        channel_bandwidth_hz: u64,
        guard_band_hz: u64,
    ) -> Self {
        let channel_spacing = channel_bandwidth_hz + guard_band_hz;
        let num_channels = ((end_frequency_hz - start_frequency_hz) / channel_spacing) as usize;

        let channels: Vec<Channel> = (0..num_channels)
            .map(|i| {
                Channel::new(
                    i,
                    start_frequency_hz + (i as u64 * channel_spacing),
                    channel_bandwidth_hz,
                )
            })
            .collect();

        Self::new(channels)
    }

    /// Update simulation time
    pub fn update_time(&mut self, time: f64) {
        self.current_time = time;
    }

    /// Allocate a channel to an agent
    /// Returns true if allocation successful
    pub fn allocate_channel(
        &mut self,
        agent_id: AgentId,
        channel_id: usize,
        tx_power_dbm: f64,
    ) -> bool {
        // Check if channel exists
        let channel = match self.channels.iter().find(|c| c.id == channel_id) {
            Some(c) => *c,
            None => return false,
        };

        // Create allocation
        let allocation = FrequencyAllocation {
            agent_id,
            channel,
            allocation_time: self.current_time,
            tx_power_dbm,
        };

        // Update tracking
        self.allocations.insert(agent_id, allocation);
        self.channel_usage
            .entry(channel_id)
            .or_insert_with(HashSet::new)
            .insert(agent_id);

        true
    }

    /// Deallocate channel from agent
    pub fn deallocate_channel(&mut self, agent_id: AgentId) {
        if let Some(allocation) = self.allocations.remove(&agent_id) {
            if let Some(users) = self.channel_usage.get_mut(&allocation.channel.id) {
                users.remove(&agent_id);
                if users.is_empty() {
                    self.channel_usage.remove(&allocation.channel.id);
                }
            }
        }
    }

    /// Get allocation for agent
    pub fn get_allocation(&self, agent_id: AgentId) -> Option<&FrequencyAllocation> {
        self.allocations.get(&agent_id)
    }

    /// Find least used channel
    pub fn find_least_used_channel(&self) -> Option<Channel> {
        self.channels
            .iter()
            .min_by_key(|c| {
                self.channel_usage
                    .get(&c.id)
                    .map(|users| users.len())
                    .unwrap_or(0)
            })
            .copied()
    }

    /// Find channel with minimum interference for agent
    /// Returns channel ID and estimated interference level
    pub fn find_best_channel(&self, _agent_id: AgentId) -> Option<(Channel, f64)> {
        let mut best_channel = None;
        let mut min_interference = f64::INFINITY;

        for channel in &self.channels {
            let interference = self.compute_channel_interference(*channel);

            if interference < min_interference {
                min_interference = interference;
                best_channel = Some(*channel);
            }
        }

        best_channel.map(|c| (c, min_interference))
    }

    /// Compute total interference on a channel
    fn compute_channel_interference(&self, channel: Channel) -> f64 {
        let mut total_interference = 0.0;

        // Count agents using this channel or overlapping channels
        for (ch_id, users) in &self.channel_usage {
            if let Some(ch) = self.channels.iter().find(|c| c.id == *ch_id) {
                if ch.overlaps(&channel) {
                    // Weight interference by number of users and frequency overlap
                    let separation = ch.frequency_separation(&channel);
                    let weight = if separation == 0 {
                        1.0 // Same channel
                    } else {
                        // Adjacent channel interference reduces with separation
                        (self.min_separation_hz as f64 / separation as f64).min(1.0)
                    };

                    total_interference += users.len() as f64 * weight;
                }
            }
        }

        total_interference
    }

    /// Get channel utilization (fraction of channels in use)
    pub fn get_utilization(&self) -> f64 {
        if self.channels.is_empty() {
            return 0.0;
        }

        let used_channels = self.channel_usage.len();
        used_channels as f64 / self.channels.len() as f64
    }

    /// Get all available channels
    pub fn get_channels(&self) -> &[Channel] {
        &self.channels
    }

    /// Get number of agents on a channel
    pub fn get_channel_load(&self, channel_id: usize) -> usize {
        self.channel_usage
            .get(&channel_id)
            .map(|users| users.len())
            .unwrap_or(0)
    }

    /// Generate frequency hopping pattern (pseudo-random sequence)
    /// Returns sequence of channel IDs
    pub fn generate_hopping_pattern(&self, seed: u64, length: usize) -> Vec<usize> {
        use std::collections::hash_map::DefaultHasher;
        use std::hash::{Hash, Hasher};

        let num_channels = self.channels.len();
        if num_channels == 0 {
            return Vec::new();
        }

        let mut pattern = Vec::with_capacity(length);
        let mut state = seed;

        for _ in 0..length {
            // Simple PRNG (Linear Congruential Generator)
            state = state.wrapping_mul(1103515245).wrapping_add(12345);

            // Hash to get better distribution
            let mut hasher = DefaultHasher::new();
            state.hash(&mut hasher);
            let hash = hasher.finish();

            let channel_idx = (hash % num_channels as u64) as usize;
            pattern.push(self.channels[channel_idx].id);
        }

        pattern
    }

    /// Check if allocation would cause excessive interference
    pub fn would_cause_interference(&self, channel: Channel, max_interference: f64) -> bool {
        let current_interference = self.compute_channel_interference(channel);
        current_interference >= max_interference
    }
}

impl Default for SpectrumManager {
    fn default() -> Self {
        // Default: 2.4 GHz ISM band with 13 channels
        Self::with_uniform_channels(
            2_400_000_000, // 2.4 GHz
            2_483_500_000, // 2.4835 GHz
            20_000_000,    // 20 MHz bandwidth
            5_000_000,     // 5 MHz guard band
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_channel_overlap() {
        let ch1 = Channel::new(0, 2_400_000_000, 20_000_000);
        let ch2 = Channel::new(1, 2_420_000_000, 20_000_000);
        let ch3 = Channel::new(2, 2_450_000_000, 20_000_000);

        assert!(ch1.overlaps(&ch2)); // Adjacent channels overlap
        assert!(!ch1.overlaps(&ch3)); // Separated channels don't overlap
    }

    #[test]
    fn test_spectrum_allocation() {
        let channels = vec![
            Channel::new(0, 2_400_000_000, 20_000_000),
            Channel::new(1, 2_420_000_000, 20_000_000),
            Channel::new(2, 2_440_000_000, 20_000_000),
        ];

        let mut manager = SpectrumManager::new(channels);

        // Allocate channels
        assert!(manager.allocate_channel(0, 0, 20.0));
        assert!(manager.allocate_channel(1, 1, 20.0));

        // Check allocations
        assert!(manager.get_allocation(0).is_some());
        assert!(manager.get_allocation(1).is_some());
        assert!(manager.get_allocation(2).is_none());

        // Check utilization
        assert!((manager.get_utilization() - 0.666).abs() < 0.01);
    }

    #[test]
    fn test_least_used_channel() {
        let channels = vec![
            Channel::new(0, 2_400_000_000, 20_000_000),
            Channel::new(1, 2_420_000_000, 20_000_000),
            Channel::new(2, 2_440_000_000, 20_000_000),
        ];

        let mut manager = SpectrumManager::new(channels);

        // Allocate two agents to channel 0
        manager.allocate_channel(0, 0, 20.0);
        manager.allocate_channel(1, 0, 20.0);

        // Allocate one agent to channel 1
        manager.allocate_channel(2, 1, 20.0);

        // Least used should be channel 2 (no users)
        let least_used = manager.find_least_used_channel();
        assert!(least_used.is_some());
        assert_eq!(least_used.unwrap().id, 2);
    }

    #[test]
    fn test_frequency_hopping() {
        let manager = SpectrumManager::default();

        let pattern = manager.generate_hopping_pattern(12345, 100);

        assert_eq!(pattern.len(), 100);

        // All channel IDs should be valid
        for ch_id in pattern {
            assert!(manager.get_channels().iter().any(|c| c.id == ch_id));
        }
    }

    #[test]
    fn test_uniform_channels() {
        let manager = SpectrumManager::with_uniform_channels(
            2_400_000_000, // Start: 2.4 GHz
            2_480_000_000, // End: 2.48 GHz
            20_000_000,    // Bandwidth: 20 MHz
            5_000_000,     // Guard: 5 MHz
        );

        let channels = manager.get_channels();
        assert!(channels.len() > 0);

        // Check channel spacing
        if channels.len() > 1 {
            let spacing = channels[1].frequency_hz - channels[0].frequency_hz;
            assert_eq!(spacing, 25_000_000); // 20 MHz + 5 MHz guard
        }
    }
}
