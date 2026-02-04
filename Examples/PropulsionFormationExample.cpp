// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

/**
 * @file PropulsionFormationExample.cpp
 * @brief Example demonstrating PropulsionModel and FormationControl usage
 *
 * This example shows how to:
 * 1. Create and configure propulsion models for different vehicle types
 * 2. Set up formation control for multiple vehicles
 * 3. Compute propulsion forces and formation commands
 */

#include "control/FormationControl.hpp"
#include "physics/PropulsionModel.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

using namespace nervosys::autonomylib;

void demonstratePropulsionModels() {
    std::cout << "\n=== PROPULSION MODEL DEMONSTRATION ===\n\n";

    // 1. UAV Rotor (Quadcopter)
    {
        std::cout << "1. UAV ROTOR (Quadcopter)\n";
        std::cout << "-------------------------\n";

        PropulsionModel::PropulsionParams rotor_params;
        rotor_params.type = PropulsionModel::PropulsionType::Rotor;
        rotor_params.diameter = 0.254f; // 10 inch rotor
        rotor_params.num_blades = 2;
        rotor_params.max_rpm = 10000.0f;
        rotor_params.efficiency = 0.85f;
        rotor_params.air_density = 1.225f; // Sea level

        PropulsionModel rotor(rotor_params);

        // Test at different throttle levels
        for (real_T throttle : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}) {
            auto output = rotor.compute(throttle, 0.0f); // Hovering
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Throttle: " << (throttle * 100) << "% -> "
                      << "Thrust: " << output.thrust << " N, "
                      << "Power: " << output.power << " W, "
                      << "RPM: " << output.rpm << "\n";
        }
        std::cout << "\n";
    }

    // 2. Marine Propeller (USV)
    {
        std::cout << "2. MARINE PROPELLER (USV)\n";
        std::cout << "-------------------------\n";

        PropulsionModel::PropulsionParams prop_params;
        prop_params.type = PropulsionModel::PropulsionType::Propeller;
        prop_params.diameter = 0.30f; // 30 cm propeller
        prop_params.pitch = 0.20f;    // 20 cm pitch
        prop_params.num_blades = 3;
        prop_params.max_rpm = 3000.0f;
        prop_params.efficiency = 0.75f;
        prop_params.air_density = 1000.0f; // Water density

        PropulsionModel propeller(prop_params);

        // Test at different speeds
        real_T throttle = 0.75f;
        for (real_T velocity : {0.0f, 1.0f, 2.0f, 3.0f, 4.0f}) {
            auto output = propeller.compute(throttle, velocity);
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Velocity: " << velocity << " m/s -> "
                      << "Thrust: " << output.thrust << " N, "
                      << "Advance Ratio J: " << output.advance_ratio << ", "
                      << "Efficiency: " << (output.efficiency * 100) << "%\n";
        }
        std::cout << "\n";
    }

    // 3. UUV Thruster
    {
        std::cout << "3. UUV THRUSTER (Underwater Vehicle)\n";
        std::cout << "------------------------------------\n";

        PropulsionModel::PropulsionParams thruster_params;
        thruster_params.type = PropulsionModel::PropulsionType::Thruster;
        thruster_params.diameter = 0.20f; // 20 cm duct diameter
        thruster_params.pitch = 0.15f;
        thruster_params.num_blades = 4;
        thruster_params.max_rpm = 2500.0f;
        thruster_params.duct_ratio = 1.3f;     // Ducted increases thrust ~30%
        thruster_params.air_density = 1025.0f; // Seawater density

        PropulsionModel thruster(thruster_params);

        real_T throttle = 1.0f;
        auto output = thruster.compute(throttle, 0.0f);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Max Thrust: " << output.thrust << " N\n";
        std::cout << "  Max Power: " << output.power << " W\n";
        std::cout << "  Note: Ducted thruster provides ~30% more thrust than open propeller\n\n";
    }

    // 4. UGV Wheel
    {
        std::cout << "4. UGV WHEEL (Ground Vehicle)\n";
        std::cout << "-----------------------------\n";

        PropulsionModel::PropulsionParams wheel_params;
        wheel_params.type = PropulsionModel::PropulsionType::Wheel;
        wheel_params.diameter = 0.30f;   // 30 cm wheel radius
        wheel_params.gear_ratio = 10.0f; // 10:1 gearbox
        wheel_params.efficiency = 0.90f;

        PropulsionModel wheel(wheel_params);

        real_T throttle = 0.5f;
        real_T angular_velocity = 10.0f; // rad/s
        auto output = wheel.compute(throttle, 0.0f, angular_velocity);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Torque: " << output.torque << " N⋅m\n";
        std::cout << "  Traction Force: " << output.thrust << " N\n";
        std::cout << "  Power: " << output.power << " W\n";
        std::cout << "  RPM: " << output.rpm << "\n\n";
    }
}

void demonstrateFormationControl() {
    std::cout << "\n=== FORMATION CONTROL DEMONSTRATION ===\n\n";

    // Setup formation parameters
    FormationControl::FormationParams params;
    params.type = FormationControl::FormationType::Wedge;
    params.spacing = 5.0f; // 5 meters between vehicles
    params.collision_radius = 2.0f;
    params.max_velocity = 10.0f;
    params.k_position = 1.0f;
    params.k_velocity = 0.5f;
    params.k_separation = 2.0f;

    FormationControl formation(params);

    // 1. Wedge Formation
    {
        std::cout << "1. WEDGE FORMATION (V-Shape)\n";
        std::cout << "----------------------------\n";

        // Leader state
        FormationControl::VehicleState leader;
        leader.position = Vector3r(0, 0, 10); // 10m altitude
        leader.velocity = Vector3r(5, 0, 0);  // Moving forward at 5 m/s
        leader.orientation = Quaternionr::Identity();
        leader.id = 0;

        // Show desired positions for 5 vehicles in wedge formation
        for (int i = 0; i < 5; ++i) {
            Vector3r desired_pos = formation.getDesiredPosition(i, leader, 5);
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Vehicle " << i << " position: (" << desired_pos.x() << ", " << desired_pos.y() << ", "
                      << desired_pos.z() << ")\n";
        }
        std::cout << "\n";
    }

    // 2. Circle Formation
    {
        std::cout << "2. CIRCLE FORMATION\n";
        std::cout << "-------------------\n";

        params.type = FormationControl::FormationType::Circle;
        params.formation_radius = 10.0f; // 10m radius circle
        formation.setParams(params);

        FormationControl::VehicleState leader;
        leader.position = Vector3r(0, 0, 15);
        leader.velocity = Vector3r(0, 0, 0); // Stationary
        leader.orientation = Quaternionr::Identity();
        leader.id = 0;

        // Show desired positions for 8 vehicles in circle formation
        for (int i = 0; i < 8; ++i) {
            Vector3r desired_pos = formation.getDesiredPosition(i, leader, 8);
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Vehicle " << i << " position: (" << desired_pos.x() << ", " << desired_pos.y() << ", "
                      << desired_pos.z() << ")\n";
        }
        std::cout << "\n";
    }

    // 3. Formation Control Commands with Collision Avoidance
    {
        std::cout << "3. FORMATION CONTROL WITH COLLISION AVOIDANCE\n";
        std::cout << "---------------------------------------------\n";

        params.type = FormationControl::FormationType::Line;
        formation.setParams(params);

        // Leader
        FormationControl::VehicleState leader;
        leader.position = Vector3r(0, 0, 10);
        leader.velocity = Vector3r(3, 0, 0);
        leader.orientation = Quaternionr::Identity();
        leader.id = 0;

        // Create 3 follower vehicles
        std::vector<FormationControl::VehicleState> all_states;
        all_states.push_back(leader);

        for (int i = 1; i <= 3; ++i) {
            FormationControl::VehicleState vehicle;
            vehicle.id = i;
            // Start slightly off position to show correction
            vehicle.position = Vector3r(-i * 4.5f, i * 4.8f, 10);
            vehicle.velocity = Vector3r(2.5, 0, 0);
            vehicle.orientation = Quaternionr::Identity();
            all_states.push_back(vehicle);
        }

        // Compute control commands for each follower
        for (int i = 1; i <= 3; ++i) {
            auto command = formation.computeCommand(i, all_states[i], all_states, leader);

            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Vehicle " << i << " commands:\n";
            std::cout << "    Current pos: (" << all_states[i].position.x() << ", " << all_states[i].position.y()
                      << ", " << all_states[i].position.z() << ")\n";
            std::cout << "    Desired vel: (" << command.desired_velocity.x() << ", " << command.desired_velocity.y()
                      << ", " << command.desired_velocity.z() << ") m/s\n";
            std::cout << "    Desired accel: (" << command.desired_acceleration.x() << ", "
                      << command.desired_acceleration.y() << ", " << command.desired_acceleration.z() << ") m/s²\n\n";
        }
    }

    // 4. Custom Formation
    {
        std::cout << "4. CUSTOM FORMATION (User-Defined Positions)\n";
        std::cout << "--------------------------------------------\n";

        // Define custom formation: square with diagonal vehicles
        std::vector<Vector3r> custom_positions = {
            Vector3r(5, 5, 0),   // Top-right
            Vector3r(-5, 5, 0),  // Top-left
            Vector3r(-5, -5, 0), // Bottom-left
            Vector3r(5, -5, 0),  // Bottom-right
            Vector3r(0, 0, 5)    // Above center
        };

        formation.setCustomFormation(custom_positions);

        FormationControl::VehicleState leader;
        leader.position = Vector3r(0, 0, 10);
        leader.velocity = Vector3r(0, 0, 0);
        leader.orientation = Quaternionr::Identity();
        leader.id = 0;

        for (int i = 0; i < 5; ++i) {
            Vector3r desired_pos = formation.getDesiredPosition(i, leader, 5);
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Vehicle " << i << " position: (" << desired_pos.x() << ", " << desired_pos.y() << ", "
                      << desired_pos.z() << ")\n";
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   AutonomySim Advanced Features Demonstration              ║\n";
    std::cout << "║   PropulsionModel & FormationControl Examples              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";

    try {
        demonstratePropulsionModels();
        demonstrateFormationControl();

        std::cout << "\n=== DEMONSTRATION COMPLETE ===\n\n";
        std::cout << "These examples show:\n";
        std::cout << "• Propulsion models for UAVs, USVs, UUVs, and UGVs\n";
        std::cout << "• Multiple formation types: Wedge, Circle, Line, Custom\n";
        std::cout << "• Collision avoidance and formation control laws\n";
        std::cout << "• Physics-based thrust, power, and efficiency calculations\n\n";

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
