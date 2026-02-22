//! Performance monitoring and metrics

use anyhow::Result;
use parking_lot::RwLock;
use serde::{Deserialize, Serialize};
use std::sync::Arc;
use std::time::{Duration, Instant};

/// SUMMONER performance metrics
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct SummonerMetrics {
    /// Total number of active agents
    pub active_agents: usize,

    /// Average step time (milliseconds)
    pub avg_step_time_ms: f64,

    /// Minimum step time (milliseconds)
    pub min_step_time_ms: f64,

    /// Maximum step time (milliseconds)
    pub max_step_time_ms: f64,

    /// Total steps executed
    pub total_steps: u64,

    /// Steps per second (throughput)
    pub steps_per_second: f64,

    /// Agents per second (throughput)
    pub agents_per_second: f64,
}

/// Performance monitor
pub struct PerformanceMonitor {
    metrics: Arc<RwLock<SummonerMetrics>>,
    step_times: Arc<RwLock<Vec<Duration>>>,
    start_time: Instant,
    _metrics_port: Option<u16>,
}

impl PerformanceMonitor {
    /// Create new performance monitor
    pub fn new(metrics_port: Option<u16>) -> Result<Self> {
        // TODO: Start Prometheus metrics server if port provided

        Ok(Self {
            metrics: Arc::new(RwLock::new(SummonerMetrics::default())),
            step_times: Arc::new(RwLock::new(Vec::new())),
            start_time: Instant::now(),
            _metrics_port: metrics_port,
        })
    }

    /// Record a completed step
    pub fn record_step(&self, step: u64, step_time: Duration, num_agents: usize) {
        let mut step_times = self.step_times.write();
        step_times.push(step_time);

        // Keep last 1000 samples
        if step_times.len() > 1000 {
            step_times.drain(0..100);
        }

        // Update metrics
        let mut metrics = self.metrics.write();
        metrics.active_agents = num_agents;
        metrics.total_steps = step + 1;

        // Compute statistics
        if !step_times.is_empty() {
            let times_ms: Vec<f64> = step_times
                .iter()
                .map(|d| d.as_secs_f64() * 1000.0)
                .collect();

            metrics.avg_step_time_ms = times_ms.iter().sum::<f64>() / times_ms.len() as f64;
            metrics.min_step_time_ms = times_ms.iter().copied().fold(f64::INFINITY, f64::min);
            metrics.max_step_time_ms = times_ms.iter().copied().fold(f64::NEG_INFINITY, f64::max);

            // Compute throughput
            let elapsed = self.start_time.elapsed().as_secs_f64();
            if elapsed > 0.0 {
                metrics.steps_per_second = metrics.total_steps as f64 / elapsed;
                metrics.agents_per_second =
                    (metrics.total_steps as f64 * num_agents as f64) / elapsed;
            }
        }
    }

    /// Get current metrics
    pub fn get_metrics(&self) -> SummonerMetrics {
        self.metrics.read().clone()
    }

    /// Print metrics summary
    pub fn print_summary(&self) {
        let metrics = self.metrics.read();

        println!("\n=== SUMMONER PERFORMANCE METRICS ===");
        println!("Active Agents:     {}", metrics.active_agents);
        println!("Total Steps:       {}", metrics.total_steps);
        println!("Avg Step Time:     {:.2} ms", metrics.avg_step_time_ms);
        println!("Min Step Time:     {:.2} ms", metrics.min_step_time_ms);
        println!("Max Step Time:     {:.2} ms", metrics.max_step_time_ms);
        println!("Steps/Second:      {:.1}", metrics.steps_per_second);
        println!("Agents/Second:     {:.0}", metrics.agents_per_second);
        println!("===================================\n");
    }
}
