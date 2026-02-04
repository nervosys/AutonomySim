// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

/**
 * @file SwarmAgenticExample.cpp
 * @brief Comprehensive example demonstrating MCP, A2A, and NANDA for collaborative swarm operations
 *
 * This example shows:
 * - Creating and configuring an agentic swarm
 * - Adding agents with different roles and capabilities
 * - MCP-based context sharing
 * - A2A communication and consensus building
 * - NANDA distributed decision-making
 * - Formation control integration
 * - Emergent swarm behaviors
 * - Collaborative mission execution
 */

#include "modules/ai/AgenticSwarmController.hpp"
#include "modules/physics/PropulsionModel.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace nervosys::autonomylib;
using namespace nervosys::autonomylib::modules;

void printHeader(const std::string &title) {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║ " << std::left << std::setw(61) << title << " ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
}

void printSection(const std::string &title) {
    std::cout << "\n─────────────────────────────────────────────────────────────────\n";
    std::cout << title << "\n";
    std::cout << "─────────────────────────────────────────────────────────────────\n";
}

int main() {
    printHeader("AUTONOMYSIM - AGENTIC SWARM COLLABORATIVE MISSION");

    try {
        // ═══════════════════════════════════════════════════════════════
        // 1. CONFIGURE SWARM CONTROLLER
        // ═══════════════════════════════════════════════════════════════
        printSection("1. Configuring Agentic Swarm Controller");

        AgenticSwarmController::SwarmConfig config;

        // MCP Configuration
        config.mcp_config.server_id = "swarm_mcp_server";
        config.mcp_config.port = 9000;
        config.mcp_config.max_agents = 50;
        config.mcp_config.context_buffer_size = 1000;
        config.mcp_config.context_timeout_sec = 10.0f;

        // A2A Configuration
        config.a2a_config.agent_id = "swarm_coordinator";
        config.a2a_config.port = 9100;
        config.a2a_config.max_connections = 50;
        config.a2a_config.heartbeat_interval_sec = 1.0f;

        // NANDA Configuration
        config.nanda_config.default_decision_mode = NANDAFramework::DecisionMode::Consensus;
        config.nanda_config.consensus_threshold = 0.7f;
        config.nanda_config.max_agents = 50;
        config.nanda_config.communication_range_meters = 1000.0f;
        config.nanda_config.enable_emergent_behavior = true;
        config.nanda_config.enable_dynamic_roles = true;

        // Formation Configuration
        config.formation_params.type = FormationControl::FormationType::Wedge;
        config.formation_params.spacing = 10.0f;
        config.formation_params.collision_radius = 5.0f;
        config.formation_params.k_position = 1.2f;
        config.formation_params.k_velocity = 0.6f;
        config.formation_params.k_separation = 2.5f;

        // Swarm Parameters
        config.min_agents = 3;
        config.max_agents = 50;
        config.update_rate_hz = 20.0f;
        config.enable_auto_recovery = true;
        config.enable_adaptive_formation = true;

        std::cout << "✓ Swarm configuration complete\n";
        std::cout << "  MCP Server: " << config.mcp_config.server_id << " (port " << config.mcp_config.port << ")\n";
        std::cout << "  A2A Protocol: port " << config.a2a_config.port << "\n";
        std::cout << "  NANDA Mode: Consensus-based decision making\n";
        std::cout << "  Formation: Wedge formation with " << config.formation_params.spacing << "m spacing\n";

        // ═══════════════════════════════════════════════════════════════
        // 2. INITIALIZE SWARM CONTROLLER
        // ═══════════════════════════════════════════════════════════════
        printSection("2. Initializing Swarm Controller");

        AgenticSwarmController swarm(config);
        swarm.start();

        std::cout << "✓ Swarm controller initialized and started\n";
        std::cout << "  State: " << static_cast<int>(swarm.getSwarmState()) << "\n";
        std::cout << "  Running: " << (swarm.isRunning() ? "Yes" : "No") << "\n";

        // ═══════════════════════════════════════════════════════════════
        // 3. CREATE AND ADD SWARM AGENTS
        // ═══════════════════════════════════════════════════════════════
        printSection("3. Creating Heterogeneous Swarm Agents");

        std::vector<AgenticSwarmController::SwarmAgent> agents;

        // Leader UAV (strong communication and decision-making)
        AgenticSwarmController::SwarmAgent leader;
        leader.agent_id = "uav_leader_001";
        leader.nanda_state.agent_id = "uav_leader_001";
        leader.nanda_state.role = NANDAFramework::AgentRole::Leader;
        leader.nanda_state.position = Vector3r(0, 0, 100);
        leader.nanda_state.velocity = Vector3r(10, 0, 0);
        leader.nanda_state.orientation = Quaternionr::Identity();
        leader.nanda_state.energy_level = 1.0f;
        leader.nanda_state.capabilities["communication"] = 0.9f;
        leader.nanda_state.capabilities["decision_making"] = 0.95f;
        leader.nanda_state.capabilities["navigation"] = 0.85f;
        leader.nanda_state.current_behavior = NANDAFramework::BehaviorType::Formation;
        leader.mcp_connected = true;
        leader.a2a_connected = true;

        agents.push_back(leader);
        swarm.addAgent(leader);
        swarm.setFormationLeader("uav_leader_001");

        std::cout << "✓ Added Leader UAV: " << leader.agent_id << "\n";
        std::cout << "  Role: Leader | Energy: " << leader.nanda_state.energy_level * 100 << "%\n";
        std::cout << "  Capabilities: Communication (90%), Decision Making (95%), Navigation (85%)\n";

        // Scout UAVs (fast, good sensors)
        for (int i = 0; i < 3; ++i) {
            AgenticSwarmController::SwarmAgent scout;
            scout.agent_id = "uav_scout_00" + std::to_string(i + 1);
            scout.nanda_state.agent_id = scout.agent_id;
            scout.nanda_state.role = NANDAFramework::AgentRole::Scout;
            scout.nanda_state.position = Vector3r(i * 20 - 20, 10, 95);
            scout.nanda_state.velocity = Vector3r(15, 0, 0);
            scout.nanda_state.orientation = Quaternionr::Identity();
            scout.nanda_state.energy_level = 0.9f;
            scout.nanda_state.capabilities["sensing"] = 0.95f;
            scout.nanda_state.capabilities["speed"] = 0.9f;
            scout.nanda_state.capabilities["navigation"] = 0.85f;
            scout.nanda_state.current_behavior = NANDAFramework::BehaviorType::Exploration;
            scout.mcp_connected = true;
            scout.a2a_connected = true;

            agents.push_back(scout);
            swarm.addAgent(scout);

            std::cout << "✓ Added Scout UAV: " << scout.agent_id << "\n";
        }

        // Worker UAVs (cargo transport, task execution)
        for (int i = 0; i < 4; ++i) {
            AgenticSwarmController::SwarmAgent worker;
            worker.agent_id = "uav_worker_00" + std::to_string(i + 1);
            worker.nanda_state.agent_id = worker.agent_id;
            worker.nanda_state.role = NANDAFramework::AgentRole::Worker;
            worker.nanda_state.position = Vector3r(i * 20 - 30, -10, 95);
            worker.nanda_state.velocity = Vector3r(10, 0, 0);
            worker.nanda_state.orientation = Quaternionr::Identity();
            worker.nanda_state.energy_level = 0.95f;
            worker.nanda_state.capabilities["payload"] = 0.9f;
            worker.nanda_state.capabilities["endurance"] = 0.85f;
            worker.nanda_state.capabilities["task_execution"] = 0.9f;
            worker.nanda_state.current_behavior = NANDAFramework::BehaviorType::Formation;
            worker.mcp_connected = true;
            worker.a2a_connected = true;

            agents.push_back(worker);
            swarm.addAgent(worker);

            std::cout << "✓ Added Worker UAV: " << worker.agent_id << "\n";
        }

        // Guardian UAV (security, monitoring)
        AgenticSwarmController::SwarmAgent guardian;
        guardian.agent_id = "uav_guardian_001";
        guardian.nanda_state.agent_id = "uav_guardian_001";
        guardian.nanda_state.role = NANDAFramework::AgentRole::Guardian;
        guardian.nanda_state.position = Vector3r(0, 20, 110);
        guardian.nanda_state.velocity = Vector3r(12, 0, 0);
        guardian.nanda_state.orientation = Quaternionr::Identity();
        guardian.nanda_state.energy_level = 0.92f;
        guardian.nanda_state.capabilities["surveillance"] = 0.95f;
        guardian.nanda_state.capabilities["threat_detection"] = 0.9f;
        guardian.nanda_state.capabilities["protection"] = 0.85f;
        guardian.nanda_state.current_behavior = NANDAFramework::BehaviorType::Defense;
        guardian.mcp_connected = true;
        guardian.a2a_connected = true;

        agents.push_back(guardian);
        swarm.addAgent(guardian);

        std::cout << "✓ Added Guardian UAV: " << guardian.agent_id << "\n";
        std::cout << "\n📊 Total Agents: " << swarm.getAgentCount() << "\n";

        // ═══════════════════════════════════════════════════════════════
        // 4. MCP CONTEXT SHARING
        // ═══════════════════════════════════════════════════════════════
        printSection("4. MCP Context Sharing & Tool Registration");

        // Publish context for each agent
        for (const auto &agent : agents) {
            MCPServer::ContextData context;
            context.agent_id = agent.agent_id;
            context.position = agent.nanda_state.position;
            context.velocity = agent.nanda_state.velocity;
            context.orientation = agent.nanda_state.orientation;
            context.mission_state = "active";
            context.perception_data["sensor_status"] = "operational";
            context.planning_data["current_goal"] = "formation_flight";
            context.execution_data["control_mode"] = "autonomous";
            context.timestamp = 1000; // Placeholder timestamp

            swarm.publishContext(context);
        }

        std::cout << "✓ Published context for " << agents.size() << " agents\n";

        // Query swarm context
        auto swarm_contexts = swarm.querySwarmContext();
        std::cout << "✓ Retrieved " << swarm_contexts.size() << " agent contexts from MCP\n";

        // Register tools
        MCPServer::Tool sensing_tool;
        sensing_tool.name = "advanced_sensing";
        sensing_tool.description = "High-resolution multi-spectral sensing";
        sensing_tool.agent_id = "uav_scout_001";
        sensing_tool.parameters = {"target_location", "sensor_mode", "resolution"};

        swarm.getMCPServer().registerTool(sensing_tool);
        std::cout << "✓ Registered sensing tool from Scout UAV\n";

        // ═══════════════════════════════════════════════════════════════
        // 5. A2A COMMUNICATION & CONSENSUS
        // ═══════════════════════════════════════════════════════════════
        printSection("5. A2A Communication & Task Negotiation");

        // Broadcast swarm coordination message
        swarm.broadcastMessage("Initiating collaborative search pattern - all agents acknowledge");
        std::cout << "✓ Broadcast coordination message to all agents\n";

        // Propose task via A2A
        A2AProtocol::Proposal search_proposal;
        search_proposal.proposal_id = "search_mission_001";
        search_proposal.proposer_id = "uav_leader_001";
        search_proposal.task_description = "Coordinate search of area (500m x 500m)";
        search_proposal.required_agents = {"uav_scout_001", "uav_scout_002", "uav_worker_001"};
        search_proposal.estimated_duration_sec = 300.0f;
        search_proposal.priority = A2AProtocol::TaskPriority::High;
        search_proposal.expiry_timestamp = 2000000; // Far future

        swarm.getA2AProtocol().proposeTask(search_proposal);
        std::cout << "✓ Leader proposed search mission\n";
        std::cout << "  Task: " << search_proposal.task_description << "\n";
        std::cout << "  Required agents: " << search_proposal.required_agents.size() << "\n";
        std::cout << "  Duration: " << search_proposal.estimated_duration_sec << " seconds\n";

        // Agents accept proposal
        for (const auto &agent_id : search_proposal.required_agents) {
            swarm.getA2AProtocol().acceptProposal(search_proposal.proposal_id, agent_id);
        }
        std::cout << "✓ All required agents accepted the proposal\n";

        // Initiate consensus on formation change
        swarm.getA2AProtocol().initiateConsensus("change_to_circle_formation", 5);
        std::cout << "✓ Initiated consensus vote on formation change\n";

        // Agents vote
        for (size_t i = 0; i < 5 && i < agents.size(); ++i) {
            swarm.getA2AProtocol().vote("consensus_" + std::to_string(1000), agents[i].agent_id, "approve");
        }
        std::cout << "✓ Collected votes from swarm members\n";

        // ═══════════════════════════════════════════════════════════════
        // 6. NANDA DISTRIBUTED DECISION-MAKING
        // ═══════════════════════════════════════════════════════════════
        printSection("6. NANDA Distributed Intelligence & Role Assignment");

        // Enable distributed decision making
        swarm.enableCollectiveDecisionMaking(true);
        swarm.enableEmergentBehaviors(true);
        swarm.enableDynamicRoleAssignment(true);

        std::cout << "✓ Enabled collective intelligence features\n";
        std::cout << "  • Collective decision-making: ON\n";
        std::cout << "  • Emergent behaviors: ON\n";
        std::cout << "  • Dynamic role assignment: ON\n";

        // Assess swarm capabilities
        auto capabilities = swarm.assessSwarmCapabilities();
        std::cout << "\n📊 Swarm Collective Capabilities:\n";
        for (const auto &cap : capabilities) {
            std::cout << "  " << cap.first << ": " << cap.second << "\n";
        }

        // Create distributed tasks
        NANDAFramework::Task reconnaissance_task;
        reconnaissance_task.task_id = "recon_001";
        reconnaissance_task.description = "Reconnaissance of target area";
        reconnaissance_task.location = Vector3r(500, 500, 100);
        reconnaissance_task.priority = 0.9f;
        reconnaissance_task.required_capabilities = {"sensing", "navigation"};
        reconnaissance_task.status = "pending";

        swarm.getNANDAFramework().createTask(reconnaissance_task);
        std::cout << "\n✓ Created reconnaissance task\n";
        std::cout << "  Location: (" << reconnaissance_task.location.x() << ", " << reconnaissance_task.location.y()
                  << ", " << reconnaissance_task.location.z() << ")\n";

        // Let NANDA allocate tasks automatically
        swarm.getNANDAFramework().allocateTasks();
        std::cout << "✓ NANDA automatically allocated tasks based on agent capabilities\n";

        // ═══════════════════════════════════════════════════════════════
        // 7. FORMATION CONTROL
        // ═══════════════════════════════════════════════════════════════
        printSection("7. Adaptive Formation Control");

        std::cout << "Current Formation: Wedge\n";

        // Get formation commands
        auto formation_commands = swarm.getFormationCommands();
        std::cout << "✓ Computed formation commands for " << formation_commands.size() << " agents\n";

        // Display formation commands for first 3 agents
        for (size_t i = 0; i < 3 && i < formation_commands.size(); ++i) {
            const auto &cmd = formation_commands[i];
            std::cout << "  Agent " << i << " - Desired velocity: (" << cmd.desired_velocity.x() << ", "
                      << cmd.desired_velocity.y() << ", " << cmd.desired_velocity.z() << ")\n";
        }

        // Change formation dynamically
        swarm.setFormation(FormationControl::FormationType::Circle);
        std::cout << "\n✓ Formation changed to Circle\n";

        formation_commands = swarm.getFormationCommands();
        std::cout << "✓ Recomputed formation commands for circular formation\n";

        // ═══════════════════════════════════════════════════════════════
        // 8. CREATE AND EXECUTE MISSION
        // ═══════════════════════════════════════════════════════════════
        printSection("8. Collaborative Mission Execution");

        AgenticSwarmController::Mission search_rescue_mission;
        search_rescue_mission.mission_id = "search_rescue_001";
        search_rescue_mission.type = AgenticSwarmController::MissionType::SearchAndRescue;
        search_rescue_mission.description = "Search and rescue operation in disaster zone";
        search_rescue_mission.target_location = Vector3r(1000, 1000, 0);
        search_rescue_mission.priority = 0.95f;
        search_rescue_mission.assigned_agents = {"uav_scout_001", "uav_scout_002", "uav_worker_001", "uav_worker_002"};
        search_rescue_mission.state = AgenticSwarmController::SwarmState::Planning;
        search_rescue_mission.parameters["search_radius"] = "500";
        search_rescue_mission.parameters["altitude"] = "80";

        std::string mission_id = swarm.createMission(search_rescue_mission);
        std::cout << "✓ Created search & rescue mission: " << mission_id << "\n";
        std::cout << "  Type: Search and Rescue\n";
        std::cout << "  Target: (" << search_rescue_mission.target_location.x() << ", "
                  << search_rescue_mission.target_location.y() << ")\n";
        std::cout << "  Assigned agents: " << search_rescue_mission.assigned_agents.size() << "\n";

        // Start mission
        swarm.startMission(mission_id);
        std::cout << "✓ Mission started - swarm executing collaboratively\n";

        // ═══════════════════════════════════════════════════════════════
        // 9. EMERGENT BEHAVIORS
        // ═══════════════════════════════════════════════════════════════
        printSection("9. Detecting Emergent Swarm Behaviors");

        // Simulate swarm update cycles
        for (int i = 0; i < 3; ++i) {
            swarm.update(0.05f); // 50ms update
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        auto emergent_behaviors = swarm.getEmergentBehaviors();
        std::cout << "✓ Detected " << emergent_behaviors.size() << " emergent behaviors\n";

        for (const auto &behavior : emergent_behaviors) {
            std::cout << "  • Behavior: " << static_cast<int>(behavior.type)
                      << " | Strength: " << behavior.strength * 100 << "%\n";
        }

        // ═══════════════════════════════════════════════════════════════
        // 10. SWARM METRICS & ANALYSIS
        // ═══════════════════════════════════════════════════════════════
        printSection("10. Swarm Intelligence Metrics");

        Vector3r centroid = swarm.getSwarmCentroid();
        real_T cohesion = swarm.getSwarmCohesion();
        real_T dispersion = swarm.getSwarmDispersion();

        std::cout << "📊 Swarm Spatial Metrics:\n";
        std::cout << "  Centroid: (" << centroid.x() << ", " << centroid.y() << ", " << centroid.z() << ")\n";
        std::cout << "  Cohesion: " << cohesion * 100 << "% (higher = more cohesive)\n";
        std::cout << "  Dispersion: " << dispersion << " meters\n";

        std::cout << "\n📊 Swarm Composition:\n";
        std::cout << "  Total agents: " << swarm.getAgentCount() << "\n";
        std::cout << "  Active missions: " << swarm.getActiveMissions().size() << "\n";
        std::cout << "  Swarm state: " << static_cast<int>(swarm.getSwarmState()) << "\n";

        // ═══════════════════════════════════════════════════════════════
        // 11. CLEANUP
        // ═══════════════════════════════════════════════════════════════
        printSection("11. Mission Complete - Cleanup");

        std::cout << "✓ Stopping swarm controller\n";
        swarm.stop();

        std::cout << "✓ All systems shutdown gracefully\n";

        printHeader("MISSION SUCCESS - COLLABORATIVE SWARM DEMONSTRATION COMPLETE");

        std::cout << "\n✅ Key Achievements:\n";
        std::cout << "  ✓ Created heterogeneous swarm with 9 agents (Leader, Scouts, Workers, Guardian)\n";
        std::cout << "  ✓ Demonstrated MCP context sharing and tool registration\n";
        std::cout << "  ✓ Performed A2A task negotiation and consensus building\n";
        std::cout << "  ✓ Utilized NANDA for distributed decision-making and role assignment\n";
        std::cout << "  ✓ Applied adaptive formation control (Wedge → Circle)\n";
        std::cout << "  ✓ Executed collaborative search & rescue mission\n";
        std::cout << "  ✓ Detected and analyzed emergent swarm behaviors\n";
        std::cout << "  ✓ Computed swarm intelligence metrics (cohesion, dispersion, capabilities)\n";

        std::cout << "\n🎯 Next Steps:\n";
        std::cout << "  • Integrate with actual vehicle physics and control systems\n";
        std::cout << "  • Add real-time visualization and monitoring\n";
        std::cout << "  • Implement obstacle avoidance and path planning\n";
        std::cout << "  • Connect to simulation environment (Unreal Engine)\n";
        std::cout << "  • Add machine learning for adaptive behaviors\n\n";

    } catch (const std::exception &e) {
        std::cerr << "\n❌ ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
