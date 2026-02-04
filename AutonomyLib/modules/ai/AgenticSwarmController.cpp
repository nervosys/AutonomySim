// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#include "modules/ai/AgenticSwarmController.hpp"
#include <algorithm>
#include <chrono>
#include <sstream>

namespace nervosys {
namespace autonomylib {
namespace modules {

AgenticSwarmController::AgenticSwarmController()
    : swarm_state_(SwarmState::Idle), running_(false), initialized_(false), formation_leader_id_("") {}

AgenticSwarmController::AgenticSwarmController(const SwarmConfig &config)
    : swarm_state_(SwarmState::Idle), running_(false), initialized_(false), formation_leader_id_("") {
    initialize(config);
}

AgenticSwarmController::~AgenticSwarmController() { stop(); }

void AgenticSwarmController::initialize(const SwarmConfig &config) {
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);
    std::lock_guard<std::mutex> lock_missions(missions_mutex_);
    std::lock_guard<std::mutex> lock_state(state_mutex_);

    config_ = config;

    // Initialize subsystems
    mcp_server_ = std::make_unique<MCPServer>(config_.mcp_config);
    a2a_protocol_ = std::make_unique<A2AProtocol>(config_.a2a_config);
    nanda_framework_ = std::make_unique<NANDAFramework>(config_.nanda_config);
    formation_control_ = std::make_unique<FormationControl>(config_.formation_params);

    agents_.clear();
    missions_.clear();
    formation_leader_id_ = "";
    swarm_state_ = SwarmState::Idle;

    initialized_ = true;
}

void AgenticSwarmController::start() {
    if (!initialized_) {
        throw std::runtime_error("AgenticSwarmController not initialized");
    }

    std::lock_guard<std::mutex> lock(state_mutex_);

    // Start all subsystems
    mcp_server_->start();
    a2a_protocol_->start();
    nanda_framework_->start();

    running_ = true;
    swarm_state_ = SwarmState::Idle;
}

void AgenticSwarmController::stop() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    running_ = false;

    if (mcp_server_)
        mcp_server_->stop();
    if (a2a_protocol_)
        a2a_protocol_->stop();
    if (nanda_framework_)
        nanda_framework_->stop();

    swarm_state_ = SwarmState::Idle;
}

void AgenticSwarmController::reset() {
    stop();

    std::lock_guard<std::mutex> lock_agents(agents_mutex_);
    std::lock_guard<std::mutex> lock_missions(missions_mutex_);

    if (mcp_server_)
        mcp_server_->reset();
    if (a2a_protocol_)
        a2a_protocol_->reset();
    if (nanda_framework_)
        nanda_framework_->reset();
    if (formation_control_)
        formation_control_->reset();

    agents_.clear();
    missions_.clear();
    formation_leader_id_ = "";
}

void AgenticSwarmController::update(real_T delta_time_sec) {
    if (!running_) {
        return;
    }

    // Update all subsystems
    synchronizeAgents();
    processDecisions();
    updateFormations();
    updateMissions();
    checkAgentHealth();
    handleEmergencies();

    // Update NANDA framework
    if (nanda_framework_) {
        nanda_framework_->update(delta_time_sec);
    }
}

// Agent management
bool AgenticSwarmController::addAgent(const SwarmAgent &agent) {
    if (!running_ || agent.agent_id.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.size() >= config_.max_agents) {
        return false;
    }

    // Register with all subsystems
    SwarmAgent new_agent = agent;
    new_agent.last_update_timestamp = getCurrentTimestamp();

    agents_[agent.agent_id] = new_agent;

    // Register with NANDA
    if (nanda_framework_) {
        nanda_framework_->registerAgent(agent.nanda_state);
    }

    // Register with MCP
    if (mcp_server_) {
        std::map<std::string, std::string> caps;
        for (const auto &cap : agent.nanda_state.capabilities) {
            caps[cap.first] = std::to_string(cap.second);
        }
        mcp_server_->registerAgent(agent.agent_id, caps);
    }

    return true;
}

bool AgenticSwarmController::removeAgent(const std::string &agent_id) {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    // Unregister from all subsystems
    if (nanda_framework_) {
        nanda_framework_->unregisterAgent(agent_id);
    }

    if (mcp_server_) {
        mcp_server_->unregisterAgent(agent_id);
    }

    return agents_.erase(agent_id) > 0;
}

bool AgenticSwarmController::updateAgent(const SwarmAgent &agent) {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    auto it = agents_.find(agent.agent_id);
    if (it == agents_.end()) {
        return false;
    }

    it->second = agent;
    it->second.last_update_timestamp = getCurrentTimestamp();

    // Update NANDA state
    if (nanda_framework_) {
        nanda_framework_->updateAgentState(agent.nanda_state);
    }

    return true;
}

AgenticSwarmController::SwarmAgent AgenticSwarmController::getAgent(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    auto it = agents_.find(agent_id);
    if (it != agents_.end()) {
        return it->second;
    }

    return SwarmAgent();
}

std::vector<AgenticSwarmController::SwarmAgent> AgenticSwarmController::getAllAgents() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    std::vector<SwarmAgent> agents;
    agents.reserve(agents_.size());

    for (const auto &pair : agents_) {
        agents.push_back(pair.second);
    }

    return agents;
}

// Mission management
std::string AgenticSwarmController::createMission(const Mission &mission) {
    if (!running_) {
        return "";
    }

    std::lock_guard<std::mutex> lock(missions_mutex_);

    Mission new_mission = mission;
    if (new_mission.mission_id.empty()) {
        new_mission.mission_id = generateMissionId();
    }
    new_mission.start_timestamp = getCurrentTimestamp();
    new_mission.state = SwarmState::Planning;

    missions_[new_mission.mission_id] = new_mission;

    // Create tasks in NANDA framework
    if (nanda_framework_) {
        for (const auto &task : new_mission.tasks) {
            nanda_framework_->createTask(task);
        }
    }

    return new_mission.mission_id;
}

bool AgenticSwarmController::startMission(const std::string &mission_id) {
    std::lock_guard<std::mutex> lock_missions(missions_mutex_);
    std::lock_guard<std::mutex> lock_state(state_mutex_);

    auto it = missions_.find(mission_id);
    if (it == missions_.end()) {
        return false;
    }

    it->second.state = SwarmState::Executing;
    swarm_state_ = SwarmState::Executing;

    return true;
}

bool AgenticSwarmController::pauseMission(const std::string &mission_id) {
    std::lock_guard<std::mutex> lock(missions_mutex_);

    auto it = missions_.find(mission_id);
    if (it == missions_.end()) {
        return false;
    }

    it->second.state = SwarmState::Idle;

    return true;
}

bool AgenticSwarmController::resumeMission(const std::string &mission_id) {
    std::lock_guard<std::mutex> lock(missions_mutex_);

    auto it = missions_.find(mission_id);
    if (it == missions_.end() || it->second.state != SwarmState::Idle) {
        return false;
    }

    it->second.state = SwarmState::Executing;

    return true;
}

bool AgenticSwarmController::abortMission(const std::string &mission_id) {
    std::lock_guard<std::mutex> lock(missions_mutex_);

    auto it = missions_.find(mission_id);
    if (it == missions_.end()) {
        return false;
    }

    it->second.state = SwarmState::Failed;

    return true;
}

AgenticSwarmController::Mission AgenticSwarmController::getMission(const std::string &mission_id) const {
    std::lock_guard<std::mutex> lock(missions_mutex_);

    auto it = missions_.find(mission_id);
    if (it != missions_.end()) {
        return it->second;
    }

    return Mission();
}

std::vector<AgenticSwarmController::Mission> AgenticSwarmController::getActiveMissions() const {
    std::lock_guard<std::mutex> lock(missions_mutex_);

    std::vector<Mission> active;

    for (const auto &pair : missions_) {
        if (pair.second.state == SwarmState::Executing || pair.second.state == SwarmState::Planning) {
            active.push_back(pair.second);
        }
    }

    return active;
}

// Formation control
bool AgenticSwarmController::setFormation(FormationControl::FormationType type) {
    if (!formation_control_) {
        return false;
    }

    formation_control_->setFormationType(type);
    return true;
}

bool AgenticSwarmController::setFormationLeader(const std::string &agent_id) {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.find(agent_id) == agents_.end()) {
        return false;
    }

    formation_leader_id_ = agent_id;
    return true;
}

FormationControl::FormationType AgenticSwarmController::getFormationType() const {
    if (!formation_control_) {
        return FormationControl::FormationType::Line;
    }

    return formation_control_->getType();
}

std::vector<FormationControl::FormationCommand> AgenticSwarmController::getFormationCommands() {
    if (!formation_control_ || formation_leader_id_.empty()) {
        return std::vector<FormationControl::FormationCommand>();
    }

    std::lock_guard<std::mutex> lock(agents_mutex_);

    // Get leader state
    auto leader_it = agents_.find(formation_leader_id_);
    if (leader_it == agents_.end()) {
        return std::vector<FormationControl::FormationCommand>();
    }

    FormationControl::VehicleState leader_state(leader_it->second.nanda_state.position,
                                                leader_it->second.nanda_state.velocity,
                                                leader_it->second.nanda_state.orientation, 0);

    // Build all vehicle states
    std::vector<FormationControl::VehicleState> all_states;
    int idx = 0;
    for (const auto &pair : agents_) {
        FormationControl::VehicleState state(pair.second.nanda_state.position, pair.second.nanda_state.velocity,
                                             pair.second.nanda_state.orientation, idx++);
        all_states.push_back(state);
    }

    // Compute commands for each vehicle
    std::vector<FormationControl::FormationCommand> commands;
    idx = 0;
    for (const auto &pair : agents_) {
        FormationControl::VehicleState current_state(pair.second.nanda_state.position, pair.second.nanda_state.velocity,
                                                     pair.second.nanda_state.orientation, idx);

        auto command = formation_control_->computeCommand(idx, current_state, all_states, leader_state);
        commands.push_back(command);
        idx++;
    }

    return commands;
}

// Swarm intelligence
void AgenticSwarmController::enableCollectiveDecisionMaking(bool enable) {
    if (nanda_framework_) {
        NANDAFramework::NANDAConfig config = nanda_framework_->getConfig();
        config.default_decision_mode =
            enable ? NANDAFramework::DecisionMode::Consensus : NANDAFramework::DecisionMode::Centralized;
        nanda_framework_->initialize(config);
    }
}

void AgenticSwarmController::enableEmergentBehaviors(bool enable) {
    if (nanda_framework_) {
        NANDAFramework::NANDAConfig config = nanda_framework_->getConfig();
        config.enable_emergent_behavior = enable;
        nanda_framework_->initialize(config);
    }
}

void AgenticSwarmController::enableDynamicRoleAssignment(bool enable) {
    if (nanda_framework_) {
        NANDAFramework::NANDAConfig config = nanda_framework_->getConfig();
        config.enable_dynamic_roles = enable;
        nanda_framework_->initialize(config);
    }
}

std::vector<NANDAFramework::EmergentBehavior> AgenticSwarmController::getEmergentBehaviors() const {
    if (!nanda_framework_) {
        return std::vector<NANDAFramework::EmergentBehavior>();
    }

    return nanda_framework_->detectEmergentBehaviors();
}

std::map<std::string, real_T> AgenticSwarmController::assessSwarmCapabilities() const {
    if (!nanda_framework_) {
        return std::map<std::string, real_T>();
    }

    return nanda_framework_->assessSwarmCapabilities();
}

// Communication
bool AgenticSwarmController::broadcastMessage(const std::string &message) {
    if (!a2a_protocol_) {
        return false;
    }

    A2AProtocol::Message msg;
    msg.message_id = "broadcast_" + std::to_string(getCurrentTimestamp());
    msg.sender_id = config_.a2a_config.agent_id;
    msg.type = A2AProtocol::MessageType::Broadcast;
    msg.content = message;
    msg.timestamp = getCurrentTimestamp();

    return a2a_protocol_->sendBroadcast(msg);
}

bool AgenticSwarmController::sendAgentMessage(const std::string &to_agent_id, const std::string &message) {
    if (!a2a_protocol_) {
        return false;
    }

    A2AProtocol::Message msg;
    msg.message_id = "msg_" + std::to_string(getCurrentTimestamp());
    msg.sender_id = config_.a2a_config.agent_id;
    msg.receiver_id = to_agent_id;
    msg.type = A2AProtocol::MessageType::Request;
    msg.content = message;
    msg.timestamp = getCurrentTimestamp();

    return a2a_protocol_->sendMessage(msg);
}

std::vector<A2AProtocol::Message> AgenticSwarmController::getMessages() {
    if (!a2a_protocol_) {
        return std::vector<A2AProtocol::Message>();
    }

    return a2a_protocol_->receiveMessages();
}

bool AgenticSwarmController::publishContext(const MCPServer::ContextData &context) {
    if (!mcp_server_) {
        return false;
    }

    return mcp_server_->publishContext(context);
}

std::vector<MCPServer::ContextData> AgenticSwarmController::querySwarmContext() const {
    if (!mcp_server_) {
        return std::vector<MCPServer::ContextData>();
    }

    return mcp_server_->queryContext();
}

// State queries
Vector3r AgenticSwarmController::getSwarmCentroid() const {
    if (!nanda_framework_) {
        return Vector3r::Zero();
    }

    return nanda_framework_->computeSwarmCentroid();
}

real_T AgenticSwarmController::getSwarmCohesion() const {
    if (!nanda_framework_) {
        return 0;
    }

    return nanda_framework_->computeSwarmCohesion();
}

real_T AgenticSwarmController::getSwarmDispersion() const {
    if (!nanda_framework_) {
        return 0;
    }

    return nanda_framework_->computeSwarmDispersion();
}

// Internal methods
void AgenticSwarmController::synchronizeAgents() {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    // Sync agent states between subsystems
    if (nanda_framework_) {
        for (auto &pair : agents_) {
            SwarmAgent &agent = pair.second;
            agent.nanda_state = nanda_framework_->getAgentState(agent.agent_id);
        }
    }

    // Publish contexts to MCP
    if (mcp_server_) {
        for (const auto &pair : agents_) {
            if (pair.second.mcp_connected) {
                mcp_server_->publishContext(pair.second.mcp_context);
            }
        }
    }
}

void AgenticSwarmController::processDecisions() {
    // NANDA framework handles decision processing internally
}

void AgenticSwarmController::updateFormations() {
    if (!config_.enable_adaptive_formation || !formation_control_) {
        return;
    }

    // Get formation commands and apply them
    // (In a real implementation, these would be sent to actual vehicles)
    getFormationCommands();
}

void AgenticSwarmController::checkAgentHealth() {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    uint64_t current_time = getCurrentTimestamp();
    uint64_t timeout_ms = static_cast<uint64_t>(config_.agent_timeout_sec * 1000);

    for (auto &pair : agents_) {
        SwarmAgent &agent = pair.second;
        if (current_time - agent.last_update_timestamp > timeout_ms) {
            // Agent timed out
            agent.mcp_connected = false;
            agent.a2a_connected = false;
        }
    }
}

void AgenticSwarmController::handleEmergencies() {
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);
    std::lock_guard<std::mutex> lock_state(state_mutex_);

    // Check for critical agent count
    if (agents_.size() < config_.min_agents) {
        swarm_state_ = SwarmState::Emergency;
        return;
    }

    // Check for low energy
    uint32_t low_energy_count = 0;
    for (const auto &pair : agents_) {
        if (pair.second.nanda_state.energy_level < 0.2f) {
            low_energy_count++;
        }
    }

    if (low_energy_count > agents_.size() / 2) {
        swarm_state_ = SwarmState::Emergency;
    }
}

void AgenticSwarmController::updateMissions() {
    std::lock_guard<std::mutex> lock(missions_mutex_);

    for (auto &pair : missions_) {
        Mission &mission = pair.second;

        if (mission.state != SwarmState::Executing) {
            continue;
        }

        // Update mission progress based on task completion
        if (nanda_framework_ && !mission.tasks.empty()) {
            real_T total_progress = 0;
            for (const auto &task : mission.tasks) {
                auto current_task = nanda_framework_->getTask(task.task_id);
                total_progress += current_task.completion_percentage;
            }
            mission.completion_percentage = total_progress / static_cast<real_T>(mission.tasks.size());

            if (mission.completion_percentage >= 1.0f) {
                mission.state = SwarmState::Completed;
            }
        }
    }
}

std::string AgenticSwarmController::generateMissionId() const {
    std::stringstream ss;
    ss << "mission_" << getCurrentTimestamp();
    return ss.str();
}

uint64_t AgenticSwarmController::getCurrentTimestamp() const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
}

} // namespace modules
} // namespace autonomylib
} // namespace nervosys
