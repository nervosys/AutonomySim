// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_modules_physics_PropulsionModel_hpp
#define autonomylib_modules_physics_PropulsionModel_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"

namespace nervosys {
namespace autonomylib {

/**
 * @brief Advanced propulsion model supporting multiple vehicle types
 *
 * Implements physics-based propulsion for:
 * - Marine vehicles (propellers with B-series coefficients)
 * - UAVs (rotors with momentum theory)
 * - UUVs (thrusters with duct effects)
 * - UGVs (wheels and tracks)
 * - Fixed-wing (jet engines)
 */
class PropulsionModel {
  public:
    enum class PropulsionType {
        Propeller, // Marine propeller (ships, boats)
        Rotor,     // UAV rotor (quadcopters, helicopters)
        Thruster,  // UUV thruster (underwater vehicles)
        Wheel,     // UGV wheel (ground vehicles)
        Track,     // UGV track (tracked vehicles)
        Jet        // Fixed-wing jet engine
    };

    struct PropulsionParams {
        PropulsionType type;
        real_T diameter;    // [m] Propeller/rotor diameter or wheel radius
        real_T pitch;       // [m] Propeller pitch
        real_T blade_area;  // [m²] Total blade area
        real_T num_blades;  // Number of blades
        real_T max_rpm;     // Maximum RPM
        real_T efficiency;  // Propulsive efficiency (0-1)
        real_T gear_ratio;  // Gear ratio for ground vehicles
        real_T duct_ratio;  // Duct area ratio for thrusters (1.0 = no duct)
        real_T air_density; // [kg/m³] Fluid density
        real_T disk_area;   // [m²] Rotor disk area (computed)

        PropulsionParams()
            : type(PropulsionType::Propeller), diameter(0.25f), pitch(0.15f), blade_area(0.02f), num_blades(4),
              max_rpm(10000), efficiency(0.85f), gear_ratio(1.0f), duct_ratio(1.0f), air_density(1.225f),
              disk_area(0.0f) {}
    };

    struct PropulsionOutput {
        real_T thrust;        // [N] Thrust force
        real_T torque;        // [N⋅m] Torque
        real_T power;         // [W] Power consumption
        real_T efficiency;    // Current efficiency
        real_T rpm;           // Current RPM
        real_T advance_ratio; // J = V/(n⋅D) for propellers

        PropulsionOutput() : thrust(0), torque(0), power(0), efficiency(0), rpm(0), advance_ratio(0) {}
    };

  public:
    PropulsionModel();
    explicit PropulsionModel(const PropulsionParams &params);

    void initialize(const PropulsionParams &params);
    void reset();

    /**
     * @brief Compute propulsion output
     * @param throttle Throttle input [0-1]
     * @param velocity Vehicle velocity [m/s]
     * @param angular_velocity Angular velocity [rad/s] (for wheels)
     * @return PropulsionOutput with thrust, torque, power, etc.
     */
    PropulsionOutput compute(real_T throttle, real_T velocity, real_T angular_velocity = 0);

    // Getters
    const PropulsionParams &getParams() const { return params_; }
    PropulsionType getType() const { return params_.type; }
    real_T getMaxThrust() const;
    real_T getMaxPower() const;

    // Setters
    void setParams(const PropulsionParams &params);
    void setFluidDensity(real_T density);

  private:
    // Type-specific computation methods
    PropulsionOutput computePropeller(real_T throttle, real_T velocity);
    PropulsionOutput computeRotor(real_T throttle, real_T velocity);
    PropulsionOutput computeThruster(real_T throttle, real_T velocity);
    PropulsionOutput computeWheel(real_T throttle, real_T angular_velocity);
    PropulsionOutput computeTrack(real_T throttle, real_T velocity);
    PropulsionOutput computeJet(real_T throttle, real_T velocity);

    // Helper methods
    real_T computeThrustCoefficient(real_T advance_ratio) const;
    real_T computeTorqueCoefficient(real_T advance_ratio) const;
    real_T computeInducedPower(real_T thrust, real_T velocity) const;
    real_T saturate(real_T value, real_T min_val, real_T max_val) const;

  private:
    PropulsionParams params_;
    bool initialized_;
};

} // namespace autonomylib
} // namespace nervosys

#endif // autonomylib_modules_physics_PropulsionModel_hpp
