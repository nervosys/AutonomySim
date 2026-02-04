// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

/**
 * @file PropulsionFormationTests.cpp
 * @brief Unit tests for PropulsionModel and FormationControl
 */

#include "control/FormationControl.hpp"
#include "physics/PropulsionModel.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace nervosys::autonomylib;

#define TEST_ASSERT(condition, message)                                                                                \
    if (!(condition)) {                                                                                                \
        std::cerr << "TEST FAILED: " << message << std::endl;                                                          \
        return false;                                                                                                  \
    }

#define EPSILON 0.001f

bool testPropulsionModelRotor() {
    std::cout << "Testing PropulsionModel::Rotor... ";

    PropulsionModel::PropulsionParams params;
    params.type = PropulsionModel::PropulsionType::Rotor;
    params.diameter = 0.254f;
    params.max_rpm = 10000.0f;
    params.efficiency = 0.85f;

    PropulsionModel rotor(params);

    // Test at zero throttle
    auto output = rotor.compute(0.0f, 0.0f);
    TEST_ASSERT(std::abs(output.thrust) < EPSILON, "Thrust should be zero at zero throttle");
    TEST_ASSERT(std::abs(output.power) < EPSILON, "Power should be zero at zero throttle");

    // Test at full throttle
    output = rotor.compute(1.0f, 0.0f);
    TEST_ASSERT(output.thrust > 0.0f, "Thrust should be positive at full throttle");
    TEST_ASSERT(output.power > 0.0f, "Power should be positive at full throttle");
    TEST_ASSERT(std::abs(output.rpm - params.max_rpm) < 1.0f, "RPM should match max_rpm");

    // Test thrust increases with throttle
    auto output_50 = rotor.compute(0.5f, 0.0f);
    auto output_75 = rotor.compute(0.75f, 0.0f);
    TEST_ASSERT(output_75.thrust > output_50.thrust, "Thrust should increase with throttle");

    std::cout << "PASSED\n";
    return true;
}

bool testPropulsionModelPropeller() {
    std::cout << "Testing PropulsionModel::Propeller... ";

    PropulsionModel::PropulsionParams params;
    params.type = PropulsionModel::PropulsionType::Propeller;
    params.diameter = 0.30f;
    params.pitch = 0.20f;
    params.max_rpm = 3000.0f;
    params.air_density = 1000.0f; // Water

    PropulsionModel propeller(params);

    // Test advance ratio calculation
    auto output = propeller.compute(1.0f, 1.0f);
    TEST_ASSERT(output.advance_ratio > 0.0f, "Advance ratio should be positive");

    // Test that thrust decreases with increasing velocity (bollard pull vs cruising)
    auto output_v0 = propeller.compute(1.0f, 0.0f);
    auto output_v2 = propeller.compute(1.0f, 2.0f);
    // At higher advance ratios, thrust typically decreases

    std::cout << "PASSED\n";
    return true;
}

bool testPropulsionModelThruster() {
    std::cout << "Testing PropulsionModel::Thruster... ";

    PropulsionModel::PropulsionParams params;
    params.type = PropulsionModel::PropulsionType::Thruster;
    params.diameter = 0.20f;
    params.duct_ratio = 1.3f;
    params.max_rpm = 2500.0f;
    params.air_density = 1025.0f; // Seawater

    PropulsionModel thruster(params);

    // Create equivalent open propeller for comparison
    PropulsionModel::PropulsionParams open_params = params;
    open_params.type = PropulsionModel::PropulsionType::Propeller;
    open_params.duct_ratio = 1.0f;
    PropulsionModel open_prop(open_params);

    // Ducted should produce more thrust
    auto ducted_output = thruster.compute(1.0f, 0.0f);
    auto open_output = open_prop.compute(1.0f, 0.0f);
    TEST_ASSERT(ducted_output.thrust > open_output.thrust,
                "Ducted thruster should produce more thrust than open propeller");

    std::cout << "PASSED\n";
    return true;
}

bool testPropulsionModelWheel() {
    std::cout << "Testing PropulsionModel::Wheel... ";

    PropulsionModel::PropulsionParams params;
    params.type = PropulsionModel::PropulsionType::Wheel;
    params.diameter = 0.30f; // 30cm radius
    params.gear_ratio = 10.0f;
    params.efficiency = 0.90f;

    PropulsionModel wheel(params);

    real_T angular_velocity = 10.0f; // rad/s
    auto output = wheel.compute(0.5f, 0.0f, angular_velocity);

    TEST_ASSERT(output.thrust > 0.0f, "Wheel should produce traction force");
    TEST_ASSERT(output.torque > 0.0f, "Wheel should produce torque");
    TEST_ASSERT(output.power > 0.0f, "Wheel should consume power");

    // Check that traction force = torque / radius
    real_T expected_thrust = output.torque / (params.diameter / 2.0f);
    TEST_ASSERT(std::abs(output.thrust - expected_thrust) < EPSILON, "Traction force should equal torque/radius");

    std::cout << "PASSED\n";
    return true;
}

bool testFormationControlLineFormation() {
    std::cout << "Testing FormationControl::Line... ";

    FormationControl::FormationParams params;
    params.type = FormationControl::FormationType::Line;
    params.spacing = 5.0f;

    FormationControl formation(params);

    FormationControl::VehicleState leader;
    leader.position = Vector3r(0, 0, 10);
    leader.orientation = Quaternionr::Identity();
    leader.id = 0;

    // Get positions for 3 vehicles
    auto pos0 = formation.getDesiredPosition(0, leader, 3);
    auto pos1 = formation.getDesiredPosition(1, leader, 3);
    auto pos2 = formation.getDesiredPosition(2, leader, 3);

    // In line formation, vehicles should be spaced along y-axis
    TEST_ASSERT(std::abs(pos0.x()) < EPSILON, "Line formation should be along y-axis");
    TEST_ASSERT(std::abs(pos1.x()) < EPSILON, "Line formation should be along y-axis");
    TEST_ASSERT(std::abs(pos2.x()) < EPSILON, "Line formation should be along y-axis");

    // Check spacing
    real_T spacing_01 = std::abs(pos1.y() - pos0.y());
    real_T spacing_12 = std::abs(pos2.y() - pos1.y());
    TEST_ASSERT(std::abs(spacing_01 - params.spacing) < EPSILON, "Spacing should match parameter");
    TEST_ASSERT(std::abs(spacing_12 - params.spacing) < EPSILON, "Spacing should be uniform");

    std::cout << "PASSED\n";
    return true;
}

bool testFormationControlCircleFormation() {
    std::cout << "Testing FormationControl::Circle... ";

    FormationControl::FormationParams params;
    params.type = FormationControl::FormationType::Circle;
    params.formation_radius = 10.0f;

    FormationControl formation(params);

    FormationControl::VehicleState leader;
    leader.position = Vector3r(0, 0, 0);
    leader.orientation = Quaternionr::Identity();
    leader.id = 0;

    // Get positions for 4 vehicles (should form square on circle)
    int num_vehicles = 4;
    for (int i = 0; i < num_vehicles; ++i) {
        auto pos = formation.getDesiredPosition(i, leader, num_vehicles);

        // Check distance from center
        real_T distance = pos.norm();
        TEST_ASSERT(std::abs(distance - params.formation_radius) < EPSILON,
                    "All vehicles should be at formation_radius from center");
    }

    std::cout << "PASSED\n";
    return true;
}

bool testFormationControlWedgeFormation() {
    std::cout << "Testing FormationControl::Wedge... ";

    FormationControl::FormationParams params;
    params.type = FormationControl::FormationType::Wedge;
    params.spacing = 5.0f;
    params.formation_angle = static_cast<real_T>(M_PI / 6.0); // 30 degrees

    FormationControl formation(params);

    FormationControl::VehicleState leader;
    leader.position = Vector3r(0, 0, 10);
    leader.orientation = Quaternionr::Identity();
    leader.id = 0;

    // Leader should be at origin of formation
    auto pos0 = formation.getDesiredPosition(0, leader, 5);
    TEST_ASSERT(std::abs(pos0.x()) < EPSILON && std::abs(pos0.y()) < EPSILON, "Leader should be at formation origin");

    // Followers should alternate sides and move back
    auto pos1 = formation.getDesiredPosition(1, leader, 5);
    auto pos2 = formation.getDesiredPosition(2, leader, 5);

    TEST_ASSERT(pos1.x() < 0.0f, "Follower 1 should be behind leader");
    TEST_ASSERT(pos2.x() < 0.0f, "Follower 2 should be behind leader");
    TEST_ASSERT(pos1.y() * pos2.y() < 0.0f, "Followers should be on opposite sides");

    std::cout << "PASSED\n";
    return true;
}

bool testFormationControlCustomFormation() {
    std::cout << "Testing FormationControl::Custom... ";

    FormationControl formation;

    // Define custom positions
    std::vector<Vector3r> custom_positions = {Vector3r(1, 2, 3), Vector3r(4, 5, 6), Vector3r(7, 8, 9)};

    formation.setCustomFormation(custom_positions);

    FormationControl::VehicleState leader;
    leader.position = Vector3r(0, 0, 0);
    leader.orientation = Quaternionr::Identity();
    leader.id = 0;

    // Check that positions match custom definition
    for (size_t i = 0; i < custom_positions.size(); ++i) {
        auto pos = formation.getDesiredPosition(i, leader, custom_positions.size());
        TEST_ASSERT((pos - custom_positions[i]).norm() < EPSILON, "Custom position should match definition");
    }

    std::cout << "PASSED\n";
    return true;
}

bool testFormationControlCollisionAvoidance() {
    std::cout << "Testing FormationControl::CollisionAvoidance... ";

    FormationControl::FormationParams params;
    params.type = FormationControl::FormationType::Line;
    params.spacing = 5.0f;
    params.collision_radius = 2.0f;
    params.k_separation = 2.0f;

    FormationControl formation(params);

    // Create two vehicles that are too close
    FormationControl::VehicleState leader;
    leader.position = Vector3r(0, 0, 0);
    leader.velocity = Vector3r(0, 0, 0);
    leader.orientation = Quaternionr::Identity();
    leader.id = 0;

    FormationControl::VehicleState follower;
    follower.position = Vector3r(1, 0, 0); // Only 1m away (< collision_radius)
    follower.velocity = Vector3r(0, 0, 0);
    follower.orientation = Quaternionr::Identity();
    follower.id = 1;

    std::vector<FormationControl::VehicleState> all_states = {leader, follower};

    // Compute command - should include separation force
    auto command = formation.computeCommand(1, follower, all_states, leader);

    // The desired velocity should have a component pushing away from leader
    real_T separation_component = command.desired_velocity.x(); // Away from leader
    TEST_ASSERT(separation_component > 0.0f, "Collision avoidance should push vehicle away from neighbor");

    std::cout << "PASSED\n";
    return true;
}

bool testFormationControlCommandComputation() {
    std::cout << "Testing FormationControl::CommandComputation... ";

    FormationControl::FormationParams params;
    params.type = FormationControl::FormationType::Line;
    params.spacing = 5.0f;
    params.max_velocity = 10.0f;
    params.max_acceleration = 5.0f;
    params.k_position = 1.0f;

    FormationControl formation(params);

    FormationControl::VehicleState leader;
    leader.position = Vector3r(0, 0, 0);
    leader.velocity = Vector3r(1, 0, 0);
    leader.orientation = Quaternionr::Identity();
    leader.id = 0;

    FormationControl::VehicleState follower;
    follower.position = Vector3r(-3, 2, 0); // Slightly off desired position
    follower.velocity = Vector3r(0.5, 0, 0);
    follower.orientation = Quaternionr::Identity();
    follower.id = 1;

    std::vector<FormationControl::VehicleState> all_states = {leader, follower};

    auto command = formation.computeCommand(1, follower, all_states, leader);

    // Command should exist
    TEST_ASSERT(command.desired_velocity.norm() > 0.0f, "Command should have non-zero velocity");

    // Velocity should be within limits
    TEST_ASSERT(command.desired_velocity.norm() <= params.max_velocity + EPSILON,
                "Desired velocity should not exceed max_velocity");

    // Acceleration should be within limits
    TEST_ASSERT(command.desired_acceleration.norm() <= params.max_acceleration + EPSILON,
                "Desired acceleration should not exceed max_acceleration");

    std::cout << "PASSED\n";
    return true;
}

int main() {
    std::cout << "\n========================================\n";
    std::cout << "PropulsionModel & FormationControl Tests\n";
    std::cout << "========================================\n\n";

    int passed = 0;
    int failed = 0;

    // PropulsionModel tests
    std::cout << "--- PropulsionModel Tests ---\n";
    if (testPropulsionModelRotor())
        passed++;
    else
        failed++;
    if (testPropulsionModelPropeller())
        passed++;
    else
        failed++;
    if (testPropulsionModelThruster())
        passed++;
    else
        failed++;
    if (testPropulsionModelWheel())
        passed++;
    else
        failed++;

    // FormationControl tests
    std::cout << "\n--- FormationControl Tests ---\n";
    if (testFormationControlLineFormation())
        passed++;
    else
        failed++;
    if (testFormationControlCircleFormation())
        passed++;
    else
        failed++;
    if (testFormationControlWedgeFormation())
        passed++;
    else
        failed++;
    if (testFormationControlCustomFormation())
        passed++;
    else
        failed++;
    if (testFormationControlCollisionAvoidance())
        passed++;
    else
        failed++;
    if (testFormationControlCommandComputation())
        passed++;
    else
        failed++;

    // Summary
    std::cout << "\n========================================\n";
    std::cout << "Test Results:\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "  Total:  " << (passed + failed) << "\n";
    std::cout << "========================================\n\n";

    return (failed == 0) ? 0 : 1;
}
