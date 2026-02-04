// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_modules_ai_NANDAFramework_hpp
#define autonomylib_modules_ai_NANDAFramework_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace nervosys {
namespace autonomylib {
namespace modules {

/**
 * @brief Networked Autonomous Navigation and Decision Architecture (NANDA)
 *
 * Distributed decision-making framework with:
 * - Collective intelligence
 * - Emergent behaviors
 * - Dynamic role assignment
 * - Swarm consensus algorithms
 * - Distributed planning
 */
class NANDAFramework {
  public:
    enum class AgentRole {
        Leader,     // Coordinates swarm activities
        Scout,      // Explores and gathers information
        Worker,     // Executes tasks
        Guardian,   // Provides security/monitoring
        Relay,      // Communication relay
        Specialist, // Specialized capability
        Adaptive    // Dynamically changes role
    };

    enum class DecisionMode {
        Centralized,  // Leader makes all decisions
        Distributed,  // Each agent decides independently
        Consensus,    // Group consensus required
        Hierarchical, // Layered decision structure
        Democratic    // Voting-based decisions
    };

    enum class BehaviorType {
        Exploration,  // Explore unknown areas
        Exploitation, // Exploit known resources
        Formation,    // Maintain formation
        Dispersion,   // Spread out
        Aggregation,  // Come together
        Migration,    // Move as group
        Defense,      // Defensive posture
        Attack,       // Offensive behavior
        Search,       // Search for targets
        Rescue        // Rescue operations
    };

    struct AgentState {
        std::string agent_id;
        AgentRole role;
        Vector3r position;
        Vector3r velocity;
        Quaternionr orientation;
        real_T energy_level;
        std::map<std::string, real_T> capabilities;
        std::vector<std::string> assigned_tasks;
        BehaviorType current_behavior;
        uint64_t timestamp;

        AgentState()
            : agent_id(""), role(AgentRole::Worker), position(Vector3r::Zero()), velocity(Vector3r::Zero()),
              orientation(Quaternionr::Identity()), energy_level(1.0f), current_behavior(BehaviorType::Formation),
              timestamp(0) {}
    };

    struct SwarmDecision {
        std::string decision_id;
        std::string description;
        DecisionMode mode;
        std::map<std::string, std::string> parameters;
        std::vector<std::string> participating_agents;
        std::map<std::string, real_T> votes; // agent_id -> confidence
        real_T consensus_threshold;
        bool finalized;
        std::string outcome;
        uint64_t timestamp;

        SwarmDecision()
            : decision_id(""), description(""), mode(DecisionMode::Consensus), consensus_threshold(0.7f),
              finalized(false), outcome(""), timestamp(0) {}
    };

    struct Task {
        std::string task_id;
        std::string description;
        Vector3r location;
        real_T priority;
        real_T estimated_duration_sec;
        std::vector<std::string> required_capabilities;
        std::vector<std::string> assigned_agents;
        std::string status; // "pending", "assigned", "in_progress", "completed", "failed"
        real_T completion_percentage;
        uint64_t deadline_timestamp;

        Task()
            : task_id(""), description(""), location(Vector3r::Zero()), priority(0.5f), estimated_duration_sec(0),
              status("pending"), completion_percentage(0), deadline_timestamp(0) {}
    };

    struct EmergentBehavior {
        std::string behavior_id;
        BehaviorType type;
        std::vector<std::string> triggering_agents;
        std::map<std::string, real_T> parameters;
        real_T strength; // How strong this behavior is (0-1)
        uint64_t start_timestamp;
        uint64_t duration_ms;

        EmergentBehavior()
            : behavior_id(""), type(BehaviorType::Formation), strength(0.5f), start_timestamp(0), duration_ms(0) {}
    };

    struct NANDAConfig {
        DecisionMode default_decision_mode;
        real_T consensus_threshold;
        real_T role_change_threshold;
        uint32_t max_agents;
        real_T communication_range_meters;
        real_T perception_range_meters;
        bool enable_emergent_behavior;
        bool enable_dynamic_roles;

        NANDAConfig()
            : default_decision_mode(DecisionMode::Distributed), consensus_threshold(0.7f), role_change_threshold(0.6f),
              max_agents(100), communication_range_meters(1000.0f), perception_range_meters(100.0f),
              enable_emergent_behavior(true), enable_dynamic_roles(true) {}
    };

  public:
    NANDAFramework();
    explicit NANDAFramework(const NANDAConfig &config);
    ~NANDAFramework();

    void initialize(const NANDAConfig &config);
    void start();
    void stop();
    void reset();
    void update(real_T delta_time_sec);

    // Agent management
    bool registerAgent(const AgentState &agent);
    bool unregisterAgent(const std::string &agent_id);
    bool updateAgentState(const AgentState &agent);
    AgentState getAgentState(const std::string &agent_id) const;
    std::vector<AgentState> getAllAgents() const;

    // Role assignment
    bool assignRole(const std::string &agent_id, AgentRole role);
    AgentRole getRole(const std::string &agent_id) const;
    std::vector<std::string> getAgentsByRole(AgentRole role) const;
    void reassignRoles(); // Dynamic role reassignment based on swarm state

    // Decision making
    std::string proposeDecision(const SwarmDecision &decision);
    bool voteOnDecision(const std::string &decision_id, const std::string &agent_id, real_T confidence);
    SwarmDecision getDecisionStatus(const std::string &decision_id) const;
    std::vector<SwarmDecision> getPendingDecisions() const;
    bool finalizeDecision(const std::string &decision_id);

    // Task management
    std::string createTask(const Task &task);
    bool assignTask(const std::string &task_id, const std::vector<std::string> &agent_ids);
    bool updateTaskProgress(const std::string &task_id, real_T completion_percentage);
    bool completeTask(const std::string &task_id);
    Task getTask(const std::string &task_id) const;
    std::vector<Task> getTasksByAgent(const std::string &agent_id) const;
    std::vector<Task> getPendingTasks() const;
    void allocateTasks(); // Automatic task allocation based on capabilities

    // Emergent behavior
    std::vector<EmergentBehavior> detectEmergentBehaviors() const;
    bool triggerBehavior(const EmergentBehavior &behavior);
    std::vector<EmergentBehavior> getActiveBehaviors() const;

    // Collective intelligence
    Vector3r computeSwarmCentroid() const;
    real_T computeSwarmCohesion() const;
    real_T computeSwarmDispersion() const;
    Vector3r computeSwarmVelocity() const;
    std::map<std::string, real_T> assessSwarmCapabilities() const;

    // Navigation and planning
    std::vector<Vector3r> planSwarmPath(const Vector3r &start, const Vector3r &goal,
                                        const std::vector<std::string> &agent_ids);
    bool executeSwarmManeuver(const std::string &maneuver_name, const std::map<std::string, real_T> &parameters);

    // State queries
    uint32_t getAgentCount() const { return static_cast<uint32_t>(agents_.size()); }
    uint32_t getTaskCount() const { return static_cast<uint32_t>(tasks_.size()); }
    uint32_t getDecisionCount() const { return static_cast<uint32_t>(decisions_.size()); }
    bool isRunning() const { return running_; }

    const NANDAConfig &getConfig() const { return config_; }

  private:
    // Internal methods
    void processDecisions();
    void evaluateEmergentBehaviors();
    void updateRoles();
    std::vector<std::string> findCapableAgents(const std::vector<std::string> &required_capabilities) const;
    real_T calculateTaskFitness(const Task &task, const AgentState &agent) const;
    std::string generateId(const std::string &prefix) const;
    uint64_t getCurrentTimestamp() const;
    real_T calculateDistance(const Vector3r &p1, const Vector3r &p2) const;

  private:
    NANDAConfig config_;
    bool running_;
    bool initialized_;

    // Data storage
    std::map<std::string, AgentState> agents_;
    std::map<std::string, SwarmDecision> decisions_;
    std::map<std::string, Task> tasks_;
    std::vector<EmergentBehavior> active_behaviors_;

    // Thread safety
    mutable std::mutex agents_mutex_;
    mutable std::mutex decisions_mutex_;
    mutable std::mutex tasks_mutex_;
    mutable std::mutex behaviors_mutex_;
};

} // namespace modules
} // namespace autonomylib
} // namespace nervosys

#endif // autonomylib_modules_ai_NANDAFramework_hpp
