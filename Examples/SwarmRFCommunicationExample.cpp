/**
 * @file SwarmRFCommunicationExample.cpp
 * @brief Example demonstrating RTX RF propagation simulation with autonomous swarm
 *
 * This example shows how to integrate the RTXRFPropagation module with the agentic
 * swarm controller to simulate realistic communication constraints in autonomous
 * vehicle missions. It demonstrates:
 * - RF link quality prediction between drones
 * - Communication-aware task allocation
 * - Coverage area analysis
 * - Interference and jamming scenarios
 * - Network topology adaptation based on signal strength
 */

#include "ai/A2AProtocol.hpp"
#include "ai/AgenticSwarmController.hpp"
#include "ai/MCPServer.hpp"
#include "common/common_utils/Utils.hpp"
#include "communication/RTXRFPropagation.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace autonomysim;

/**
 * @brief Helper class to integrate RF propagation with swarm controller
 */
class RFAwareSwarmController {
  public:
    RFAwareSwarmController() : rf_propagation_(), swarm_controller_() {}

    bool initialize() {
        // Initialize RF propagation simulation
        RTXRFPropagation::RTXConfig rf_config;
        rf_config.enable_rtx = true; // Enable RTX if available
        rf_config.model = RTXRFPropagation::PropagationModel::RTX_RayTracing;
        rf_config.weather = RTXRFPropagation::WeatherCondition::Clear;
        rf_config.max_ray_bounces = 3;
        rf_config.rays_per_query = 32;
        rf_config.enable_atmospheric = true;
        rf_config.enable_ground_reflection = true;
        rf_config.enable_diffraction = true;

        if (!rf_propagation_.initialize(rf_config)) {
            Utils::log("Failed to initialize RF propagation", Utils::kLogLevelError);
            return false;
        }

        // Initialize swarm controller
        AgenticSwarmController::SwarmConfig swarm_config;
        swarm_config.max_agents = 10;
        swarm_config.communication_range = 1000.0f; // meters
        swarm_config.enable_mcp = true;
        swarm_config.enable_a2a = true;
        swarm_config.enable_nanda = true;

        if (!swarm_controller_.initialize(swarm_config)) {
            Utils::log("Failed to initialize swarm controller", Utils::kLogLevelError);
            return false;
        }

        Utils::log("RF-aware swarm controller initialized successfully", Utils::kLogLevelInfo);
        return true;
    }

    /**
     * @brief Register drone with both swarm and RF systems
     */
    void registerDrone(const std::string &drone_id, const Vector3r &position, real_T altitude) {
        // Register with swarm controller
        AgenticSwarmController::AgentInfo agent_info;
        agent_info.agent_id = drone_id;
        agent_info.vehicle_type = AgenticSwarmController::VehicleType::Multirotor;
        agent_info.initial_position = position;
        swarm_controller_.registerAgent(agent_info);

        // Register as RF transmitter
        RTXRFPropagation::Transmitter tx;
        tx.id = drone_id + "_tx";
        tx.position = position;
        tx.frequency_hz = 2.4e9; // 2.4 GHz (typical drone comms)
        tx.power_dbm = 20.0;     // 20 dBm (100 mW)
        tx.antenna_gain_dbi = 2.0;
        tx.beamwidth_degrees = 360.0; // Omnidirectional
        tx.antenna_height_m = altitude;
        tx.band = RTXRFPropagation::RFBand::SHF_3_30GHz;
        tx.active = true;
        rf_propagation_.registerTransmitter(tx);

        // Register as RF receiver
        RTXRFPropagation::Receiver rx;
        rx.id = drone_id + "_rx";
        rx.position = position;
        rx.sensitivity_dbm = -90.0; // -90 dBm typical
        rx.antenna_gain_dbi = 2.0;
        rx.noise_figure_db = 6.0;
        rx.active = true;
        rf_propagation_.registerReceiver(rx);

        Utils::log("Registered drone: " + drone_id, Utils::kLogLevelInfo);
    }

    /**
     * @brief Update drone position (both swarm and RF)
     */
    void updateDronePosition(const std::string &drone_id, const Vector3r &position, const Vector3r &velocity) {
        // Update in swarm controller
        swarm_controller_.updateAgentState(drone_id, position, velocity, Quaternionr::Identity());

        // Update RF transmitter/receiver positions
        rf_propagation_.updateTransmitter(drone_id + "_tx", position);
        rf_propagation_.updateReceiver(drone_id + "_rx", position);

        // Update velocities for Doppler shift calculation
        rf_propagation_.setTransmitterVelocity(drone_id + "_tx", velocity);
        rf_propagation_.setReceiverVelocity(drone_id + "_rx", velocity);
    }

    /**
     * @brief Check if two drones can communicate
     */
    bool canCommunicate(const std::string &drone1_id, const std::string &drone2_id, real_T &rssi_dbm, real_T &snr_db) {
        std::string tx_id = drone1_id + "_tx";
        std::string rx_id = drone2_id + "_rx";

        RTXRFPropagation::PropagationResult result = rf_propagation_.computePropagation(tx_id, rx_id);

        rssi_dbm = result.received_power_dbm;
        snr_db = result.signal_to_noise_db;

        // Consider link viable if SNR > 10 dB
        return (snr_db > 10.0 && result.line_of_sight);
    }

    /**
     * @brief Get all viable communication links
     */
    std::vector<std::pair<std::string, std::string>> getViableLinks() {
        std::vector<std::pair<std::string, std::string>> links;
        auto results = rf_propagation_.computeAllLinks();

        for (const auto &result : results) {
            if (result.signal_to_noise_db > 10.0) {
                // Extract drone IDs from transmitter/receiver IDs
                std::string drone1_id = result.status.substr(0, result.status.find("_tx"));
                std::string drone2_id = result.status.substr(result.status.find("_rx") + 3);
                links.push_back({drone1_id, drone2_id});
            }
        }

        return links;
    }

    /**
     * @brief Compute coverage area for a drone
     */
    void analyzeCoverage(const std::string &drone_id, real_T min_rssi_dbm = -90.0) {
        std::string tx_id = drone_id + "_tx";
        std::vector<Vector3r> coverage_points = rf_propagation_.computeCoverageArea(tx_id, min_rssi_dbm, 10.0);

        Utils::log("Drone " + drone_id + " coverage: " + std::to_string(coverage_points.size()) + " points",
                   Utils::kLogLevelInfo);
    }

    /**
     * @brief Add RF jammer to scenario
     */
    void addJammer(const Vector3r &position, real_T power_dbm) {
        RTXRFPropagation::Transmitter jammer;
        jammer.id = "jammer_" + std::to_string(jammer_count_++);
        jammer.position = position;
        jammer.frequency_hz = 2.4e9;
        jammer.power_dbm = power_dbm;
        jammer.antenna_gain_dbi = 6.0; // Directional jammer
        jammer.beamwidth_degrees = 60.0;
        jammer.antenna_height_m = 0.0;
        jammer.band = RTXRFPropagation::RFBand::SHF_3_30GHz;
        jammer.active = true;

        rf_propagation_.registerJammer(jammer);
        Utils::log("Added jammer at position (" + std::to_string(position.x()) + ", " + std::to_string(position.y()) +
                       ", " + std::to_string(position.z()) + ")",
                   Utils::kLogLevelInfo);
    }

    /**
     * @brief Print RF statistics
     */
    void printStatistics() {
        Utils::log("=== RF Propagation Statistics ===", Utils::kLogLevelInfo);
        Utils::log("Average compute time: " + std::to_string(rf_propagation_.getAverageComputeTimeMs()) + " ms",
                   Utils::kLogLevelInfo);

        auto links = getViableLinks();
        Utils::log("Viable communication links: " + std::to_string(links.size()), Utils::kLogLevelInfo);
    }

  private:
    RTXRFPropagation rf_propagation_;
    AgenticSwarmController swarm_controller_;
    uint32_t jammer_count_ = 0;
};

/**
 * @brief Main example function
 */
int main(int argc, char *argv[]) {
    Utils::log("=== RTX RF Propagation Swarm Example ===", Utils::kLogLevelInfo);

    // Create RF-aware swarm controller
    RFAwareSwarmController swarm;
    if (!swarm.initialize()) {
        Utils::log("Failed to initialize swarm", Utils::kLogLevelError);
        return 1;
    }

    // Scenario 1: Urban Communication Test
    Utils::log("\n=== Scenario 1: Urban Communication Test ===", Utils::kLogLevelInfo);

    // Register 4 drones in urban environment
    swarm.registerDrone("drone1", Vector3r(0, 0, 0), 50.0);
    swarm.registerDrone("drone2", Vector3r(100, 0, 0), 50.0);
    swarm.registerDrone("drone3", Vector3r(0, 100, 0), 50.0);
    swarm.registerDrone("drone4", Vector3r(100, 100, 0), 50.0);

    // Check communication between drones
    real_T rssi, snr;
    if (swarm.canCommunicate("drone1", "drone2", rssi, snr)) {
        Utils::log("Drone1 <-> Drone2: RSSI = " + std::to_string(rssi) + " dBm, SNR = " + std::to_string(snr) + " dB",
                   Utils::kLogLevelInfo);
    }

    if (swarm.canCommunicate("drone1", "drone4", rssi, snr)) {
        Utils::log("Drone1 <-> Drone4: RSSI = " + std::to_string(rssi) + " dBm, SNR = " + std::to_string(snr) + " dB",
                   Utils::kLogLevelInfo);
    }

    // Analyze coverage
    swarm.analyzeCoverage("drone1");

    // Scenario 2: Formation Flight with Dynamic Topology
    Utils::log("\n=== Scenario 2: Formation Flight ===", Utils::kLogLevelInfo);

    // Update positions (formation moving)
    for (int t = 0; t < 10; ++t) {
        real_T offset = t * 10.0;
        swarm.updateDronePosition("drone1", Vector3r(offset, 0, 0), Vector3r(10, 0, 0));
        swarm.updateDronePosition("drone2", Vector3r(offset + 100, 0, 0), Vector3r(10, 0, 0));
        swarm.updateDronePosition("drone3", Vector3r(offset, 100, 0), Vector3r(10, 0, 0));
        swarm.updateDronePosition("drone4", Vector3r(offset + 100, 100, 0), Vector3r(10, 0, 0));

        Utils::log("Time step " + std::to_string(t) + ": Formation at offset " + std::to_string(offset) + " m",
                   Utils::kLogLevelInfo);
    }

    // Scenario 3: Jamming Scenario
    Utils::log("\n=== Scenario 3: Jamming Scenario ===", Utils::kLogLevelInfo);

    // Add hostile jammer
    swarm.addJammer(Vector3r(50, 50, 0), 30.0); // 30 dBm jammer (1W)

    // Re-check communication with jamming
    if (swarm.canCommunicate("drone1", "drone2", rssi, snr)) {
        Utils::log("With jamming - Drone1 <-> Drone2: RSSI = " + std::to_string(rssi) +
                       " dBm, SNR = " + std::to_string(snr) + " dB",
                   Utils::kLogLevelInfo);
    } else {
        Utils::log("With jamming - Drone1 <-> Drone2: LINK DEGRADED", Utils::kLogLevelWarn);
    }

    // Scenario 4: Long-Range Relay
    Utils::log("\n=== Scenario 4: Long-Range Relay ===", Utils::kLogLevelInfo);

    // Add relay drone
    swarm.registerDrone("relay1", Vector3r(500, 0, 0), 100.0); // Higher altitude for better LOS

    if (swarm.canCommunicate("drone1", "relay1", rssi, snr)) {
        Utils::log("Drone1 <-> Relay: RSSI = " + std::to_string(rssi) + " dBm, SNR = " + std::to_string(snr) + " dB",
                   Utils::kLogLevelInfo);
    }

    // Print final statistics
    swarm.printStatistics();

    Utils::log("\n=== Example Complete ===", Utils::kLogLevelInfo);
    return 0;
}
