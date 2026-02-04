// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_modules_control_FormationControl_hpp
#define autonomylib_modules_control_FormationControl_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include <vector>

namespace nervosys {
namespace autonomylib {

/**
 * @brief Multi-vehicle formation control system
 *
 * Implements formation control for multiple unmanned vehicles with:
 * - Multiple formation geometries (line, wedge, circle, etc.)
 * - Collision avoidance
 * - Leader-follower control
 * - Virtual structure approach
 */
class FormationControl {
  public:
    enum class FormationType {
        Line,    // Vehicles in a line
        Column,  // Vehicles in a column
        Wedge,   // V-formation
        Diamond, // Diamond formation
        Circle,  // Circular formation
        Box,     // Box/square formation
        Custom   // User-defined formation
    };

    struct VehicleState {
        Vector3r position;
        Vector3r velocity;
        Quaternionr orientation;
        int id;

        VehicleState()
            : position(Vector3r::Zero()), velocity(Vector3r::Zero()), orientation(Quaternionr::Identity()), id(0) {}

        VehicleState(const Vector3r &pos, const Vector3r &vel, const Quaternionr &quat, int vehicle_id)
            : position(pos), velocity(vel), orientation(quat), id(vehicle_id) {}
    };

    struct FormationParams {
        FormationType type;
        real_T spacing;          // [m] Distance between vehicles
        real_T collision_radius; // [m] Minimum separation distance
        real_T max_velocity;     // [m/s] Maximum velocity
        real_T max_acceleration; // [m/s²] Maximum acceleration

        // Control gains
        real_T k_position;   // Position error gain
        real_T k_velocity;   // Velocity error gain
        real_T k_separation; // Separation gain (repulsion)
        real_T k_cohesion;   // Cohesion gain (attraction)
        real_T k_alignment;  // Alignment gain

        // Formation-specific parameters
        real_T formation_radius; // [m] Radius for circular formations
        real_T formation_angle;  // [rad] Angle for wedge formations

        FormationParams()
            : type(FormationType::Line), spacing(5.0f), collision_radius(2.0f), max_velocity(10.0f),
              max_acceleration(5.0f), k_position(1.0f), k_velocity(0.5f), k_separation(2.0f), k_cohesion(0.3f),
              k_alignment(0.2f), formation_radius(10.0f), formation_angle(static_cast<real_T>(M_PI / 6.0)) {}
    };

    struct FormationCommand {
        Vector3r desired_velocity;
        Vector3r desired_acceleration;
        Quaternionr desired_orientation;

        FormationCommand()
            : desired_velocity(Vector3r::Zero()), desired_acceleration(Vector3r::Zero()),
              desired_orientation(Quaternionr::Identity()) {}
    };

  public:
    FormationControl();
    explicit FormationControl(const FormationParams &params);

    void initialize(const FormationParams &params);
    void reset();

    /**
     * @brief Compute formation control command for a vehicle
     * @param vehicle_id ID of the vehicle
     * @param current_state Current state of the vehicle
     * @param all_states States of all vehicles in the formation
     * @param leader_state State of the leader (for leader-follower mode)
     * @return FormationCommand with desired velocity and orientation
     */
    FormationCommand computeCommand(int vehicle_id, const VehicleState &current_state,
                                    const std::vector<VehicleState> &all_states, const VehicleState &leader_state);

    /**
     * @brief Set custom formation positions relative to leader
     * @param positions Vector of relative positions for each vehicle
     */
    void setCustomFormation(const std::vector<Vector3r> &positions);

    /**
     * @brief Get desired position for a vehicle in formation
     * @param vehicle_id ID of the vehicle
     * @param leader_state State of the formation leader
     * @param num_vehicles Total number of vehicles
     * @return Desired position in world frame
     */
    Vector3r getDesiredPosition(int vehicle_id, const VehicleState &leader_state, int num_vehicles) const;

    // Getters
    const FormationParams &getParams() const { return params_; }
    FormationType getType() const { return params_.type; }

    // Setters
    void setParams(const FormationParams &params);
    void setFormationType(FormationType type);
    void setSpacing(real_T spacing);

  private:
    // Formation geometry methods
    Vector3r computeLineFormation(int index, int total) const;
    Vector3r computeColumnFormation(int index, int total) const;
    Vector3r computeWedgeFormation(int index, int total) const;
    Vector3r computeDiamondFormation(int index, int total) const;
    Vector3r computeCircleFormation(int index, int total) const;
    Vector3r computeBoxFormation(int index, int total) const;

    // Control law components
    Vector3r computePositionError(const Vector3r &current_pos, const Vector3r &desired_pos) const;

    Vector3r computeVelocityError(const Vector3r &current_vel, const Vector3r &desired_vel) const;

    Vector3r computeSeparationForce(const VehicleState &vehicle, const std::vector<VehicleState> &neighbors) const;

    Vector3r computeCohesionForce(const VehicleState &vehicle, const std::vector<VehicleState> &neighbors) const;

    Vector3r computeAlignmentForce(const VehicleState &vehicle, const std::vector<VehicleState> &neighbors) const;

    // Utility methods
    Vector3r saturateVector(const Vector3r &vec, real_T max_magnitude) const;
    Vector3r rotateVector(const Vector3r &vec, const Quaternionr &quat) const;
    real_T saturate(real_T value, real_T min_val, real_T max_val) const;

  private:
    FormationParams params_;
    std::vector<Vector3r> custom_positions_;
    bool initialized_;
};

} // namespace autonomylib
} // namespace nervosys

#endif // autonomylib_modules_control_FormationControl_hpp
