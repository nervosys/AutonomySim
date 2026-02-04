// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_modules_ai_AgenticSwarmController_hpp
#define autonomylib_modules_ai_AgenticSwarmController_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "modules/ai/NANDAFramework.hpp"
#include "modules/communication/A2AProtocol.hpp"
#include "modules/communication/MCPServer.hpp"
#include "modules/control/FormationControl.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace nervosys {
namespace autonomylib {
namespace modules {

/**
 * @brief Integrated Agentic Swarm Controller
 *
 * High-level controller that integrates:
 * - MCP (Model Context Protocol) for context sharing
 * - A2A (Agent-to-Agent) for direct communication
 * - NANDA for distributed decision-making
 * - FormationControl for spatial organization
 *
 * Enables sophisticated swarm behaviors including:
 * - Autonomous task allocation and execution
 * - Collective decision-making
 * - Emergent swarm intelligence
 * - Adaptive formation control
 * - Multi-agent coordination
 */
class AgenticSwarmController {
  public:
    enum class MissionType {
        Exploration,     // Explore unknown areas
        SearchAndRescue, // Search for targets and provide assistance
        Surveillance,    // Monitor area continuously
        Transport,       // Transport cargo/people
        Construction,    // Build or modify structures
        Defense,         // Defensive operations
        Attack,          // Offensive operations
        Reconnaissance,  // Gather intelligence
        Patrol,          // Regular patrol routes
        Custom           // User-defined mission
    };

    enum class SwarmState {
        Initializing, // Setting up swarm
        Idle,         // Waiting for commands
        Planning,     // Planning mission
        Executing,    // Executing mission
        Adapting,     // Adapting to changes
        Emergency,    // Emergency state
        Completed,    // Mission completed
        Failed        // Mission failed
    };

    struct SwarmAgent {
        std::string agent_id;
        NANDAFramework::AgentState nanda_state;
        MCPServer::ContextData mcp_context;
        bool mcp_connected;
        bool a2a_connected;
        uint64_t last_update_timestamp;

        SwarmAgent() : agent_id(""), mcp_connected(false), a2a_connected(false), last_update_timestamp(0) {}
    };

    struct Mission {
        std::string mission_id;
        MissionType type;
        std::string description;
        Vector3r target_location;
        real_T priority;
        std::vector<std::string> assigned_agents;
        std::vector<NANDAFramework::Task> tasks;
        SwarmState state;
        real_T completion_percentage;
        uint64_t start_timestamp;
        uint64_t deadline_timestamp;
        std::map<std::string, std::string> parameters;

        Mission()
            : mission_id(""), type(MissionType::Custom), description(""), target_location(Vector3r::Zero()),
              priority(0.5f), state(SwarmState::Initializing), completion_percentage(0), start_timestamp(0),
              deadline_timestamp(0) {}
    };

    struct SwarmConfig {
        // MCP configuration
        MCPServer::ServerConfig mcp_config;

        // A2A configuration
        A2AProtocol::ProtocolConfig a2a_config;

        // NANDA configuration
        NANDAFramework::NANDAConfig nanda_config;

        // Formation configuration
        FormationControl::FormationParams formation_params;

        // Swarm parameters
        uint32_t min_agents;
        uint32_t max_agents;
        real_T update_rate_hz;
        bool enable_auto_recovery;
        bool enable_adaptive_formation;
        real_T agent_timeout_sec;

        SwarmConfig()
            : min_agents(2), max_agents(100), update_rate_hz(10.0f), enable_auto_recovery(true),
              enable_adaptive_formation(true), agent_timeout_sec(5.0f) {}
    };

  public:
    AgenticSwarmController();
    explicit AgenticSwarmController(const SwarmConfig &config);
    ~AgenticSwarmController();

    void initialize(const SwarmConfig &config);
    void start();
    void stop();
    void reset();
    void update(real_T delta_time_sec);

    // Agent management
    bool addAgent(const SwarmAgent &agent);
    bool removeAgent(const std::string &agent_id);
    bool updateAgent(const SwarmAgent &agent);
    SwarmAgent getAgent(const std::string &agent_id) const;
    std::vector<SwarmAgent> getAllAgents() const;
    uint32_t getAgentCount() const { return static_cast<uint32_t>(agents_.size()); }

    // Mission management
    std::string createMission(const Mission &mission);
    bool startMission(const std::string &mission_id);
    bool pauseMission(const std::string &mission_id);
    bool resumeMission(const std::string &mission_id);
    bool abortMission(const std::string &mission_id);
    Mission getMission(const std::string &mission_id) const;
    std::vector<Mission> getActiveMissions() const;

    // Formation control
    bool setFormation(FormationControl::FormationType type);
    bool setFormationLeader(const std::string &agent_id);
    FormationControl::FormationType getFormationType() const;
    std::vector<FormationControl::FormationCommand> getFormationCommands();

    // Swarm intelligence
    void enableCollectiveDecisionMaking(bool enable);
    void enableEmergentBehaviors(bool enable);
    void enableDynamicRoleAssignment(bool enable);
    std::vector<NANDAFramework::EmergentBehavior> getEmergentBehaviors() const;
    std::map<std::string, real_T> assessSwarmCapabilities() const;

    // Communication
    bool broadcastMessage(const std::string &message);
    bool sendAgentMessage(const std::string &to_agent_id, const std::string &message);
    std::vector<A2AProtocol::Message> getMessages();
    bool publishContext(const MCPServer::ContextData &context);
    std::vector<MCPServer::ContextData> querySwarmContext() const;

    // State queries
    SwarmState getSwarmState() const { return swarm_state_; }
    bool isRunning() const { return running_; }
    Vector3r getSwarmCentroid() const;
    real_T getSwarmCohesion() const;
    real_T getSwarmDispersion() const;

    // Direct access to subsystems (for advanced usage)
    MCPServer &getMCPServer() { return *mcp_server_; }
    A2AProtocol &getA2AProtocol() { return *a2a_protocol_; }
    NANDAFramework &getNANDAFramework() { return *nanda_framework_; }
    FormationControl &getFormationControl() { return *formation_control_; }

    const SwarmConfig &getConfig() const { return config_; }

  private:
    // Internal methods
    void synchronizeAgents();
    void processDecisions();
    void updateFormations();
    void checkAgentHealth();
    void handleEmergencies();
    void updateMissions();
    std::string generateMissionId() const;
    uint64_t getCurrentTimestamp() const;

  private:
    SwarmConfig config_;
    SwarmState swarm_state_;
    bool running_;
    bool initialized_;

    // Core subsystems
    std::unique_ptr<MCPServer> mcp_server_;
    std::unique_ptr<A2AProtocol> a2a_protocol_;
    std::unique_ptr<NANDAFramework> nanda_framework_;
    std::unique_ptr<FormationControl> formation_control_;

    // Swarm data
    std::map<std::string, SwarmAgent> agents_;
    std::map<std::string, Mission> missions_;
    std::string formation_leader_id_;

    // Thread safety
    mutable std::mutex agents_mutex_;
    mutable std::mutex missions_mutex_;
    mutable std::mutex state_mutex_;
};

} // namespace modules
} // namespace autonomylib
} // namespace nervosys

#endif // autonomylib_modules_ai_AgenticSwarmController_hpp
