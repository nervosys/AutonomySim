// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#include "modules/ai/NANDAFramework.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <sstream>

namespace nervosys {
namespace autonomylib {
namespace modules {

NANDAFramework::NANDAFramework() : running_(false), initialized_(false) {}

NANDAFramework::NANDAFramework(const NANDAConfig &config) : running_(false), initialized_(false) { initialize(config); }

NANDAFramework::~NANDAFramework() { stop(); }

void NANDAFramework::initialize(const NANDAConfig &config) {
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);
    std::lock_guard<std::mutex> lock_decisions(decisions_mutex_);
    std::lock_guard<std::mutex> lock_tasks(tasks_mutex_);
    std::lock_guard<std::mutex> lock_behaviors(behaviors_mutex_);

    config_ = config;
    agents_.clear();
    decisions_.clear();
    tasks_.clear();
    active_behaviors_.clear();

    initialized_ = true;
}

void NANDAFramework::start() {
    if (!initialized_) {
        throw std::runtime_error("NANDAFramework not initialized");
    }
    running_ = true;
}

void NANDAFramework::stop() { running_ = false; }

void NANDAFramework::reset() {
    stop();
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);
    std::lock_guard<std::mutex> lock_decisions(decisions_mutex_);
    std::lock_guard<std::mutex> lock_tasks(tasks_mutex_);
    std::lock_guard<std::mutex> lock_behaviors(behaviors_mutex_);

    agents_.clear();
    decisions_.clear();
    tasks_.clear();
    active_behaviors_.clear();
}

void NANDAFramework::update(real_T delta_time_sec) {
    if (!running_) {
        return;
    }

    (void)delta_time_sec; // May be used in future implementations

    processDecisions();

    if (config_.enable_dynamic_roles) {
        updateRoles();
    }

    if (config_.enable_emergent_behavior) {
        evaluateEmergentBehaviors();
    }

    allocateTasks();
}

// Agent management
bool NANDAFramework::registerAgent(const AgentState &agent) {
    if (!running_ || agent.agent_id.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.size() >= config_.max_agents) {
        return false;
    }

    agents_[agent.agent_id] = agent;
    return true;
}

bool NANDAFramework::unregisterAgent(const std::string &agent_id) {
    std::lock_guard<std::mutex> lock(agents_mutex_);
    return agents_.erase(agent_id) > 0;
}

bool NANDAFramework::updateAgentState(const AgentState &agent) {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    auto it = agents_.find(agent.agent_id);
    if (it == agents_.end()) {
        return false;
    }

    it->second = agent;
    return true;
}

NANDAFramework::AgentState NANDAFramework::getAgentState(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    auto it = agents_.find(agent_id);
    if (it != agents_.end()) {
        return it->second;
    }

    return AgentState();
}

std::vector<NANDAFramework::AgentState> NANDAFramework::getAllAgents() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    std::vector<AgentState> agents;
    agents.reserve(agents_.size());

    for (const auto &pair : agents_) {
        agents.push_back(pair.second);
    }

    return agents;
}

// Role assignment
bool NANDAFramework::assignRole(const std::string &agent_id, AgentRole role) {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    auto it = agents_.find(agent_id);
    if (it == agents_.end()) {
        return false;
    }

    it->second.role = role;
    return true;
}

NANDAFramework::AgentRole NANDAFramework::getRole(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    auto it = agents_.find(agent_id);
    if (it != agents_.end()) {
        return it->second.role;
    }

    return AgentRole::Worker;
}

std::vector<std::string> NANDAFramework::getAgentsByRole(AgentRole role) const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    std::vector<std::string> agent_ids;

    for (const auto &pair : agents_) {
        if (pair.second.role == role) {
            agent_ids.push_back(pair.first);
        }
    }

    return agent_ids;
}

void NANDAFramework::reassignRoles() {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.empty()) {
        return;
    }

    // Simple heuristic: assign roles based on agent count
    // 1 Leader per 10 agents
    // 20% Scouts
    // 60% Workers
    // 10% Guardians
    // 10% Relays

    uint32_t count = static_cast<uint32_t>(agents_.size());
    uint32_t leader_count = std::max(1u, count / 10);
    uint32_t scout_count = count / 5;
    uint32_t guardian_count = count / 10;
    uint32_t relay_count = count / 10;

    uint32_t assigned = 0;

    // Assign leaders (highest energy)
    std::vector<std::pair<std::string, real_T>> energy_sorted;
    for (const auto &pair : agents_) {
        energy_sorted.push_back({pair.first, pair.second.energy_level});
    }
    std::sort(energy_sorted.begin(), energy_sorted.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });

    for (uint32_t i = 0; i < leader_count && i < energy_sorted.size(); ++i) {
        agents_[energy_sorted[i].first].role = AgentRole::Leader;
        assigned++;
    }

    // Assign scouts, guardians, relays
    for (uint32_t i = assigned; i < energy_sorted.size(); ++i) {
        if (i < assigned + scout_count) {
            agents_[energy_sorted[i].first].role = AgentRole::Scout;
        } else if (i < assigned + scout_count + guardian_count) {
            agents_[energy_sorted[i].first].role = AgentRole::Guardian;
        } else if (i < assigned + scout_count + guardian_count + relay_count) {
            agents_[energy_sorted[i].first].role = AgentRole::Relay;
        } else {
            agents_[energy_sorted[i].first].role = AgentRole::Worker;
        }
    }
}

// Decision making
std::string NANDAFramework::proposeDecision(const SwarmDecision &decision) {
    if (!running_) {
        return "";
    }

    std::lock_guard<std::mutex> lock(decisions_mutex_);

    SwarmDecision new_decision = decision;
    if (new_decision.decision_id.empty()) {
        new_decision.decision_id = generateId("decision");
    }
    new_decision.timestamp = getCurrentTimestamp();
    new_decision.finalized = false;

    decisions_[new_decision.decision_id] = new_decision;

    return new_decision.decision_id;
}

bool NANDAFramework::voteOnDecision(const std::string &decision_id, const std::string &agent_id, real_T confidence) {
    std::lock_guard<std::mutex> lock(decisions_mutex_);

    auto it = decisions_.find(decision_id);
    if (it == decisions_.end() || it->second.finalized) {
        return false;
    }

    it->second.votes[agent_id] = confidence;

    return true;
}

NANDAFramework::SwarmDecision NANDAFramework::getDecisionStatus(const std::string &decision_id) const {
    std::lock_guard<std::mutex> lock(decisions_mutex_);

    auto it = decisions_.find(decision_id);
    if (it != decisions_.end()) {
        return it->second;
    }

    return SwarmDecision();
}

std::vector<NANDAFramework::SwarmDecision> NANDAFramework::getPendingDecisions() const {
    std::lock_guard<std::mutex> lock(decisions_mutex_);

    std::vector<SwarmDecision> pending;

    for (const auto &pair : decisions_) {
        if (!pair.second.finalized) {
            pending.push_back(pair.second);
        }
    }

    return pending;
}

bool NANDAFramework::finalizeDecision(const std::string &decision_id) {
    std::lock_guard<std::mutex> lock(decisions_mutex_);

    auto it = decisions_.find(decision_id);
    if (it == decisions_.end()) {
        return false;
    }

    SwarmDecision &decision = it->second;

    // Calculate consensus
    if (decision.votes.empty()) {
        return false;
    }

    real_T total_confidence = 0;
    for (const auto &vote : decision.votes) {
        total_confidence += vote.second;
    }
    real_T avg_confidence = total_confidence / static_cast<real_T>(decision.votes.size());

    if (avg_confidence >= decision.consensus_threshold) {
        decision.finalized = true;
        decision.outcome = "approved";
        return true;
    }

    decision.finalized = true;
    decision.outcome = "rejected";
    return false;
}

// Task management
std::string NANDAFramework::createTask(const Task &task) {
    if (!running_) {
        return "";
    }

    std::lock_guard<std::mutex> lock(tasks_mutex_);

    Task new_task = task;
    if (new_task.task_id.empty()) {
        new_task.task_id = generateId("task");
    }

    tasks_[new_task.task_id] = new_task;

    return new_task.task_id;
}

bool NANDAFramework::assignTask(const std::string &task_id, const std::vector<std::string> &agent_ids) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }

    it->second.assigned_agents = agent_ids;
    it->second.status = "assigned";

    return true;
}

bool NANDAFramework::updateTaskProgress(const std::string &task_id, real_T completion_percentage) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }

    it->second.completion_percentage = completion_percentage;
    it->second.status = "in_progress";

    return true;
}

bool NANDAFramework::completeTask(const std::string &task_id) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }

    it->second.completion_percentage = 1.0f;
    it->second.status = "completed";

    return true;
}

NANDAFramework::Task NANDAFramework::getTask(const std::string &task_id) const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        return it->second;
    }

    return Task();
}

std::vector<NANDAFramework::Task> NANDAFramework::getTasksByAgent(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    std::vector<Task> agent_tasks;

    for (const auto &pair : tasks_) {
        const Task &task = pair.second;
        if (std::find(task.assigned_agents.begin(), task.assigned_agents.end(), agent_id) !=
            task.assigned_agents.end()) {
            agent_tasks.push_back(task);
        }
    }

    return agent_tasks;
}

std::vector<NANDAFramework::Task> NANDAFramework::getPendingTasks() const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);

    std::vector<Task> pending;

    for (const auto &pair : tasks_) {
        if (pair.second.status == "pending") {
            pending.push_back(pair.second);
        }
    }

    return pending;
}

void NANDAFramework::allocateTasks() {
    std::lock_guard<std::mutex> lock_tasks(tasks_mutex_);
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);

    // Find pending tasks
    std::vector<std::string> pending_task_ids;
    for (const auto &pair : tasks_) {
        if (pair.second.status == "pending") {
            pending_task_ids.push_back(pair.first);
        }
    }

    // Allocate each task to best-fit agents
    for (const std::string &task_id : pending_task_ids) {
        Task &task = tasks_[task_id];

        // Find capable agents
        std::vector<std::pair<std::string, real_T>> agent_fitness;
        for (const auto &agent_pair : agents_) {
            real_T fitness = calculateTaskFitness(task, agent_pair.second);
            if (fitness > 0) {
                agent_fitness.push_back({agent_pair.first, fitness});
            }
        }

        if (!agent_fitness.empty()) {
            // Sort by fitness
            std::sort(agent_fitness.begin(), agent_fitness.end(),
                      [](const auto &a, const auto &b) { return a.second > b.second; });

            // Assign best agent
            task.assigned_agents = {agent_fitness[0].first};
            task.status = "assigned";
        }
    }
}

// Emergent behavior
std::vector<NANDAFramework::EmergentBehavior> NANDAFramework::detectEmergentBehaviors() const {
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);
    std::lock_guard<std::mutex> lock_behaviors(behaviors_mutex_);

    std::vector<EmergentBehavior> detected;

    if (agents_.size() < 3) {
        return detected;
    }

    // Detect aggregation behavior (agents clustering together)
    real_T dispersion = computeSwarmDispersion();
    if (dispersion < 10.0f) { // Threshold for clustering
        EmergentBehavior behavior;
        behavior.behavior_id = generateId("aggregation");
        behavior.type = BehaviorType::Aggregation;
        behavior.strength = 1.0f - (dispersion / 10.0f);
        behavior.start_timestamp = getCurrentTimestamp();
        detected.push_back(behavior);
    }

    // Detect formation behavior (organized structure)
    real_T cohesion = computeSwarmCohesion();
    if (cohesion > 0.7f) { // High cohesion indicates formation
        EmergentBehavior behavior;
        behavior.behavior_id = generateId("formation");
        behavior.type = BehaviorType::Formation;
        behavior.strength = cohesion;
        behavior.start_timestamp = getCurrentTimestamp();
        detected.push_back(behavior);
    }

    return detected;
}

bool NANDAFramework::triggerBehavior(const EmergentBehavior &behavior) {
    if (!running_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(behaviors_mutex_);

    active_behaviors_.push_back(behavior);

    return true;
}

std::vector<NANDAFramework::EmergentBehavior> NANDAFramework::getActiveBehaviors() const {
    std::lock_guard<std::mutex> lock(behaviors_mutex_);

    uint64_t current_time = getCurrentTimestamp();
    std::vector<EmergentBehavior> active;

    for (const auto &behavior : active_behaviors_) {
        if (current_time - behavior.start_timestamp < behavior.duration_ms) {
            active.push_back(behavior);
        }
    }

    return active;
}

// Collective intelligence
Vector3r NANDAFramework::computeSwarmCentroid() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.empty()) {
        return Vector3r::Zero();
    }

    Vector3r centroid = Vector3r::Zero();
    for (const auto &pair : agents_) {
        centroid += pair.second.position;
    }

    return centroid / static_cast<real_T>(agents_.size());
}

real_T NANDAFramework::computeSwarmCohesion() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.size() < 2) {
        return 0;
    }

    Vector3r centroid = computeSwarmCentroid();

    real_T total_distance = 0;
    for (const auto &pair : agents_) {
        total_distance += calculateDistance(pair.second.position, centroid);
    }

    real_T avg_distance = total_distance / static_cast<real_T>(agents_.size());

    // Cohesion = 1 / (1 + avg_distance) (normalized to 0-1)
    return 1.0f / (1.0f + avg_distance * 0.1f);
}

real_T NANDAFramework::computeSwarmDispersion() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.size() < 2) {
        return 0;
    }

    Vector3r centroid = computeSwarmCentroid();

    real_T sum_squared_distances = 0;
    for (const auto &pair : agents_) {
        real_T dist = calculateDistance(pair.second.position, centroid);
        sum_squared_distances += dist * dist;
    }

    return std::sqrt(sum_squared_distances / static_cast<real_T>(agents_.size()));
}

Vector3r NANDAFramework::computeSwarmVelocity() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.empty()) {
        return Vector3r::Zero();
    }

    Vector3r avg_velocity = Vector3r::Zero();
    for (const auto &pair : agents_) {
        avg_velocity += pair.second.velocity;
    }

    return avg_velocity / static_cast<real_T>(agents_.size());
}

std::map<std::string, real_T> NANDAFramework::assessSwarmCapabilities() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    std::map<std::string, real_T> capabilities;

    for (const auto &pair : agents_) {
        for (const auto &cap : pair.second.capabilities) {
            capabilities[cap.first] += cap.second;
        }
    }

    return capabilities;
}

// Navigation and planning
std::vector<Vector3r> NANDAFramework::planSwarmPath(const Vector3r &start, const Vector3r &goal,
                                                    const std::vector<std::string> &agent_ids) {
    // Simple straight-line path for now
    // In a real implementation, this would use pathfinding algorithms
    (void)agent_ids; // May be used in future implementations

    std::vector<Vector3r> path;
    path.push_back(start);
    path.push_back(goal);

    return path;
}

bool NANDAFramework::executeSwarmManeuver(const std::string &maneuver_name,
                                          const std::map<std::string, real_T> &parameters) {
    (void)maneuver_name;
    (void)parameters;

    // Placeholder for swarm maneuver execution
    return running_;
}

// Internal methods
void NANDAFramework::processDecisions() {
    std::lock_guard<std::mutex> lock(decisions_mutex_);

    // Auto-finalize decisions based on mode
    for (auto &pair : decisions_) {
        SwarmDecision &decision = pair.second;

        if (decision.finalized) {
            continue;
        }

        switch (decision.mode) {
        case DecisionMode::Centralized:
            // Leader decides
            if (!decision.votes.empty()) {
                decision.finalized = true;
                decision.outcome = "approved";
            }
            break;

        case DecisionMode::Distributed:
            // Each agent decides independently
            decision.finalized = true;
            decision.outcome = "distributed";
            break;

        case DecisionMode::Consensus:
            // Check if consensus reached
            if (decision.votes.size() >= decision.participating_agents.size()) {
                finalizeDecision(decision.decision_id);
            }
            break;

        default:
            break;
        }
    }
}

void NANDAFramework::evaluateEmergentBehaviors() {
    auto detected = detectEmergentBehaviors();

    for (const auto &behavior : detected) {
        triggerBehavior(behavior);
    }
}

void NANDAFramework::updateRoles() { reassignRoles(); }

std::vector<std::string>
NANDAFramework::findCapableAgents(const std::vector<std::string> &required_capabilities) const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    std::vector<std::string> capable;

    for (const auto &pair : agents_) {
        const AgentState &agent = pair.second;

        bool has_all = true;
        for (const std::string &cap : required_capabilities) {
            if (agent.capabilities.find(cap) == agent.capabilities.end()) {
                has_all = false;
                break;
            }
        }

        if (has_all) {
            capable.push_back(agent.agent_id);
        }
    }

    return capable;
}

real_T NANDAFramework::calculateTaskFitness(const Task &task, const AgentState &agent) const {
    real_T fitness = 0;

    // Check capabilities
    for (const std::string &req_cap : task.required_capabilities) {
        auto it = agent.capabilities.find(req_cap);
        if (it != agent.capabilities.end()) {
            fitness += it->second;
        } else {
            return 0; // Missing required capability
        }
    }

    // Consider distance to task
    real_T distance = calculateDistance(agent.position, task.location);
    real_T distance_factor = 1.0f / (1.0f + distance * 0.01f);
    fitness *= distance_factor;

    // Consider energy level
    fitness *= agent.energy_level;

    // Consider current workload
    real_T workload_factor = 1.0f / (1.0f + static_cast<real_T>(agent.assigned_tasks.size()));
    fitness *= workload_factor;

    return fitness;
}

std::string NANDAFramework::generateId(const std::string &prefix) const {
    std::stringstream ss;
    ss << prefix << "_" << getCurrentTimestamp();
    return ss.str();
}

uint64_t NANDAFramework::getCurrentTimestamp() const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
}

real_T NANDAFramework::calculateDistance(const Vector3r &p1, const Vector3r &p2) const {
    Vector3r diff = p1 - p2;
    return diff.norm();
}

} // namespace modules
} // namespace autonomylib
} // namespace nervosys
