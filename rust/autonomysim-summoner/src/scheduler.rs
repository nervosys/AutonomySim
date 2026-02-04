//! Task scheduler for worker load balancing

use std::collections::VecDeque;

/// Task type for distributed execution
#[derive(Debug, Clone)]
pub enum Task {
    /// Simulate physics for agent range
    Physics {
        agent_start: usize,
        agent_end: usize,
    },

    /// Simulate sensors for agent range
    Sensors {
        agent_start: usize,
        agent_end: usize,
    },

    /// Simulate communications
    Communications {
        agent_start: usize,
        agent_end: usize,
    },

    /// Run AI inference
    AI {
        agent_start: usize,
        agent_end: usize,
    },
}

/// Work-stealing scheduler for dynamic load balancing
pub struct Scheduler {
    task_queue: VecDeque<Task>,
    num_workers: usize,
}

impl Scheduler {
    /// Create new scheduler
    pub fn new(num_workers: usize) -> Self {
        Self {
            task_queue: VecDeque::new(),
            num_workers,
        }
    }

    /// Add task to queue
    pub fn submit(&mut self, task: Task) {
        self.task_queue.push_back(task);
    }

    /// Get next task (work stealing)
    pub fn get_task(&mut self) -> Option<Task> {
        self.task_queue.pop_front()
    }

    /// Check if scheduler is idle
    pub fn is_idle(&self) -> bool {
        self.task_queue.is_empty()
    }

    /// Get queue length
    pub fn queue_len(&self) -> usize {
        self.task_queue.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_scheduler() {
        let mut scheduler = Scheduler::new(4);

        scheduler.submit(Task::Physics {
            agent_start: 0,
            agent_end: 100,
        });
        scheduler.submit(Task::Sensors {
            agent_start: 0,
            agent_end: 100,
        });

        assert_eq!(scheduler.queue_len(), 2);
        assert!(!scheduler.is_idle());

        let task = scheduler.get_task();
        assert!(task.is_some());
        assert_eq!(scheduler.queue_len(), 1);
    }
}
