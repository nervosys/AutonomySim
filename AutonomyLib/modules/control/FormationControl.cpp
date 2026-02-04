// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#include "modules/control/FormationControl.hpp"
#include <algorithm>
#include <cmath>

namespace nervosys {
namespace autonomylib {

FormationControl::FormationControl() : initialized_(false) {}

FormationControl::FormationControl(const FormationParams &params) : initialized_(false) { initialize(params); }

void FormationControl::initialize(const FormationParams &params) {
    params_ = params;
    custom_positions_.clear();
    initialized_ = true;
}

void FormationControl::reset() { custom_positions_.clear(); }

FormationControl::FormationCommand FormationControl::computeCommand(int vehicle_id, const VehicleState &current_state,
                                                                    const std::vector<VehicleState> &all_states,
                                                                    const VehicleState &leader_state) {
    FormationCommand command;

    if (!initialized_ || all_states.empty()) {
        return command;
    }

    // Compute desired position in formation
    Vector3r desired_pos = getDesiredPosition(vehicle_id, leader_state, static_cast<int>(all_states.size()));

    // Position error control
    Vector3r pos_error = computePositionError(current_state.position, desired_pos);
    Vector3r pos_control = params_.k_position * pos_error;

    // Velocity error control (match leader velocity)
    Vector3r vel_error = computeVelocityError(current_state.velocity, leader_state.velocity);
    Vector3r vel_control = params_.k_velocity * vel_error;

    // Separation force (collision avoidance)
    Vector3r separation = computeSeparationForce(current_state, all_states);
    separation = params_.k_separation * separation;

    // Cohesion force (stay with group)
    Vector3r cohesion = computeCohesionForce(current_state, all_states);
    cohesion = params_.k_cohesion * cohesion;

    // Alignment force (match neighbor velocities)
    Vector3r alignment = computeAlignmentForce(current_state, all_states);
    alignment = params_.k_alignment * alignment;

    // Combine all forces
    Vector3r total_force = pos_control + vel_control + separation + cohesion + alignment;

    // Convert to desired velocity
    command.desired_velocity = current_state.velocity + total_force;
    command.desired_velocity = saturateVector(command.desired_velocity, params_.max_velocity);

    // Desired acceleration
    command.desired_acceleration = total_force;
    command.desired_acceleration = saturateVector(command.desired_acceleration, params_.max_acceleration);

    // Desired orientation (point in direction of desired velocity)
    if (command.desired_velocity.norm() > 0.1f) {
        Vector3r forward = command.desired_velocity.normalized();
        Vector3r up(0, 0, 1);
        Vector3r right = forward.cross(up).normalized();
        up = right.cross(forward).normalized();

        // Create rotation matrix
        Matrix3x3r rotation;
        rotation.col(0) = forward;
        rotation.col(1) = right;
        rotation.col(2) = up;

        command.desired_orientation = Quaternionr(rotation);
    } else {
        command.desired_orientation = current_state.orientation;
    }

    return command;
}

void FormationControl::setCustomFormation(const std::vector<Vector3r> &positions) {
    custom_positions_ = positions;
    params_.type = FormationType::Custom;
}

Vector3r FormationControl::getDesiredPosition(int vehicle_id, const VehicleState &leader_state,
                                              int num_vehicles) const {
    if (!initialized_) {
        return Vector3r::Zero();
    }

    // Get formation offset in formation frame
    Vector3r offset;

    switch (params_.type) {
    case FormationType::Line:
        offset = computeLineFormation(vehicle_id, num_vehicles);
        break;
    case FormationType::Column:
        offset = computeColumnFormation(vehicle_id, num_vehicles);
        break;
    case FormationType::Wedge:
        offset = computeWedgeFormation(vehicle_id, num_vehicles);
        break;
    case FormationType::Diamond:
        offset = computeDiamondFormation(vehicle_id, num_vehicles);
        break;
    case FormationType::Circle:
        offset = computeCircleFormation(vehicle_id, num_vehicles);
        break;
    case FormationType::Box:
        offset = computeBoxFormation(vehicle_id, num_vehicles);
        break;
    case FormationType::Custom:
        if (vehicle_id < static_cast<int>(custom_positions_.size())) {
            offset = custom_positions_[vehicle_id];
        } else {
            offset = Vector3r::Zero();
        }
        break;
    default:
        offset = Vector3r::Zero();
    }

    // Rotate offset by leader orientation and add to leader position
    Vector3r rotated_offset = rotateVector(offset, leader_state.orientation);
    return leader_state.position + rotated_offset;
}

Vector3r FormationControl::computeLineFormation(int index, int total) const {
    (void)total; // Unused in line formation
    // Vehicles arranged in a line along the y-axis
    real_T y_offset = (index - total / 2.0f) * params_.spacing;
    return Vector3r(0, y_offset, 0);
}

Vector3r FormationControl::computeColumnFormation(int index, int total) const {
    (void)total; // Unused in column formation
    // Vehicles arranged in a column along the x-axis (behind leader)
    real_T x_offset = -index * params_.spacing;
    return Vector3r(x_offset, 0, 0);
}

Vector3r FormationControl::computeWedgeFormation(int index, int total) const {
    // V-formation
    if (index == 0) {
        return Vector3r::Zero(); // Leader at front
    }

    int side = (index % 2 == 0) ? 1 : -1; // Alternate sides
    int row = (index + 1) / 2;

    real_T x_offset = -row * params_.spacing * std::cos(params_.formation_angle);
    real_T y_offset = side * row * params_.spacing * std::sin(params_.formation_angle);

    return Vector3r(x_offset, y_offset, 0);
}

Vector3r FormationControl::computeDiamondFormation(int index, int total) const {
    // Diamond shape
    if (total < 4) {
        return computeBoxFormation(index, total);
    }

    switch (index) {
    case 0: // Front
        return Vector3r(params_.spacing, 0, 0);
    case 1: // Right
        return Vector3r(0, params_.spacing, 0);
    case 2: // Back
        return Vector3r(-params_.spacing, 0, 0);
    case 3: // Left
        return Vector3r(0, -params_.spacing, 0);
    default: // Additional vehicles in circle
        return computeCircleFormation(index - 4, total - 4);
    }
}

Vector3r FormationControl::computeCircleFormation(int index, int total) const {
    // Circular formation
    if (total <= 1) {
        return Vector3r::Zero();
    }

    real_T angle = 2.0f * static_cast<real_T>(M_PI) * index / total;
    real_T x = params_.formation_radius * std::cos(angle);
    real_T y = params_.formation_radius * std::sin(angle);

    return Vector3r(x, y, 0);
}

Vector3r FormationControl::computeBoxFormation(int index, int total) const {
    // Box/square formation
    int side_length = static_cast<int>(std::ceil(std::sqrt(static_cast<real_T>(total))));
    int row = index / side_length;
    int col = index % side_length;

    real_T x = (row - side_length / 2.0f) * params_.spacing;
    real_T y = (col - side_length / 2.0f) * params_.spacing;

    return Vector3r(x, y, 0);
}

Vector3r FormationControl::computePositionError(const Vector3r &current_pos, const Vector3r &desired_pos) const {
    return desired_pos - current_pos;
}

Vector3r FormationControl::computeVelocityError(const Vector3r &current_vel, const Vector3r &desired_vel) const {
    return desired_vel - current_vel;
}

Vector3r FormationControl::computeSeparationForce(const VehicleState &vehicle,
                                                  const std::vector<VehicleState> &neighbors) const {
    Vector3r separation = Vector3r::Zero();

    for (const auto &neighbor : neighbors) {
        if (neighbor.id == vehicle.id) {
            continue;
        }

        Vector3r diff = vehicle.position - neighbor.position;
        real_T distance = diff.norm();

        // Apply repulsion force if too close
        if (distance < params_.collision_radius && distance > 0.01f) {
            // Inverse square law for repulsion
            real_T magnitude = 1.0f / (distance * distance);
            separation += diff.normalized() * magnitude;
        }
    }

    return separation;
}

Vector3r FormationControl::computeCohesionForce(const VehicleState &vehicle,
                                                const std::vector<VehicleState> &neighbors) const {
    if (neighbors.empty()) {
        return Vector3r::Zero();
    }

    // Compute center of mass of neighbors
    Vector3r center_of_mass = Vector3r::Zero();
    int count = 0;

    for (const auto &neighbor : neighbors) {
        if (neighbor.id != vehicle.id) {
            center_of_mass += neighbor.position;
            count++;
        }
    }

    if (count == 0) {
        return Vector3r::Zero();
    }

    center_of_mass /= static_cast<float>(count);

    // Steer towards center of mass
    return center_of_mass - vehicle.position;
}

Vector3r FormationControl::computeAlignmentForce(const VehicleState &vehicle,
                                                 const std::vector<VehicleState> &neighbors) const {
    if (neighbors.empty()) {
        return Vector3r::Zero();
    }

    // Compute average velocity of neighbors
    Vector3r avg_velocity = Vector3r::Zero();
    int count = 0;

    for (const auto &neighbor : neighbors) {
        if (neighbor.id != vehicle.id) {
            avg_velocity += neighbor.velocity;
            count++;
        }
    }

    if (count == 0) {
        return Vector3r::Zero();
    }

    avg_velocity /= static_cast<float>(count);

    // Steer towards average velocity
    return avg_velocity - vehicle.velocity;
}

void FormationControl::setParams(const FormationParams &params) { initialize(params); }

void FormationControl::setFormationType(FormationType type) { params_.type = type; }

void FormationControl::setSpacing(real_T spacing) { params_.spacing = spacing; }

Vector3r FormationControl::saturateVector(const Vector3r &vec, real_T max_magnitude) const {
    real_T magnitude = vec.norm();
    if (magnitude > max_magnitude && magnitude > 0.001f) {
        return vec * (max_magnitude / magnitude);
    }
    return vec;
}

Vector3r FormationControl::rotateVector(const Vector3r &vec, const Quaternionr &quat) const { return quat * vec; }

real_T FormationControl::saturate(real_T value, real_T min_val, real_T max_val) const {
    return std::max(min_val, std::min(max_val, value));
}

} // namespace autonomylib
} // namespace nervosys
