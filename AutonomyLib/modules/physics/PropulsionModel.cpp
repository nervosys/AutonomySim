// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#include "modules/physics/PropulsionModel.hpp"
#include <algorithm>
#include <cmath>

namespace nervosys {
namespace autonomylib {

PropulsionModel::PropulsionModel() : initialized_(false) {}

PropulsionModel::PropulsionModel(const PropulsionParams &params) : initialized_(false) { initialize(params); }

void PropulsionModel::initialize(const PropulsionParams &params) {
    params_ = params;

    // Compute derived parameters
    if (params_.type == PropulsionType::Rotor || params_.type == PropulsionType::Propeller) {
        params_.disk_area = static_cast<real_T>(M_PI * params_.diameter * params_.diameter / 4.0);
    }

    initialized_ = true;
}

void PropulsionModel::reset() {
    // Reset any internal state if needed
}

PropulsionModel::PropulsionOutput PropulsionModel::compute(real_T throttle, real_T velocity, real_T angular_velocity) {
    if (!initialized_) {
        return PropulsionOutput();
    }

    // Clamp throttle to [0, 1]
    throttle = saturate(throttle, 0.0f, 1.0f);

    // Dispatch to type-specific computation
    switch (params_.type) {
    case PropulsionType::Propeller:
        return computePropeller(throttle, velocity);
    case PropulsionType::Rotor:
        return computeRotor(throttle, velocity);
    case PropulsionType::Thruster:
        return computeThruster(throttle, velocity);
    case PropulsionType::Wheel:
        return computeWheel(throttle, angular_velocity);
    case PropulsionType::Track:
        return computeTrack(throttle, velocity);
    case PropulsionType::Jet:
        return computeJet(throttle, velocity);
    default:
        return PropulsionOutput();
    }
}

PropulsionModel::PropulsionOutput PropulsionModel::computePropeller(real_T throttle, real_T velocity) {
    PropulsionOutput output;

    // RPM based on throttle
    output.rpm = throttle * params_.max_rpm;
    real_T n = output.rpm / 60.0f; // Convert to rev/s

    if (n < 0.01f) {
        return output;
    }

    // Advance ratio: J = V / (n * D)
    output.advance_ratio = velocity / (n * params_.diameter + 1e-6f);

    // B-series thrust coefficient (simplified Wageningen B-series)
    real_T Kt = computeThrustCoefficient(output.advance_ratio);
    real_T Kq = computeTorqueCoefficient(output.advance_ratio);

    // Thrust: T = Kt * ρ * n² * D⁴
    output.thrust = Kt * params_.air_density * n * n * std::pow(params_.diameter, 4.0f);

    // Torque: Q = Kq * ρ * n² * D⁵
    output.torque = Kq * params_.air_density * n * n * std::pow(params_.diameter, 5.0f);

    // Power: P = 2π * n * Q
    output.power = 2.0f * static_cast<real_T>(M_PI) * n * output.torque;

    // Efficiency: η = (T * V) / P
    if (output.power > 0.001f) {
        output.efficiency = (output.thrust * velocity) / output.power;
        output.efficiency = saturate(output.efficiency, 0.0f, 1.0f);
    }

    return output;
}

PropulsionModel::PropulsionOutput PropulsionModel::computeRotor(real_T throttle, real_T velocity) {
    PropulsionOutput output;

    // RPM based on throttle
    output.rpm = throttle * params_.max_rpm;
    real_T omega = output.rpm * 2.0f * static_cast<real_T>(M_PI) / 60.0f; // rad/s

    if (omega < 0.1f) {
        return output;
    }

    // Momentum theory for rotors
    // Thrust = 2 * ρ * A * (ΔV)²
    // where ΔV is the induced velocity

    // Simplified: assume thrust proportional to ω²
    real_T max_thrust =
        0.5f * params_.air_density * params_.disk_area * std::pow(omega * params_.diameter / 2.0f, 2.0f);

    output.thrust = throttle * throttle * max_thrust * params_.efficiency;

    // Induced power: P_ind = T^(3/2) / sqrt(2 * ρ * A)
    output.power = computeInducedPower(output.thrust, velocity);

    // Profile power (drag): approximately 10-20% of induced power
    output.power *= 1.15f;

    // Torque: Q = P / ω
    if (omega > 0.1f) {
        output.torque = output.power / omega;
    }

    // Efficiency
    if (output.power > 0.001f) {
        output.efficiency = (output.thrust * velocity) / output.power;
        output.efficiency = saturate(output.efficiency, 0.0f, params_.efficiency);
    } else {
        output.efficiency = params_.efficiency;
    }

    return output;
}

PropulsionModel::PropulsionOutput PropulsionModel::computeThruster(real_T throttle, real_T velocity) {
    // UUV thruster (essentially a ducted propeller)
    PropulsionOutput output = computePropeller(throttle, velocity);

    // Duct effect: increases thrust by ~30% for ducted thrusters
    real_T duct_factor = 1.0f + (params_.duct_ratio - 1.0f) * 0.3f;
    output.thrust *= duct_factor;

    // Slightly higher efficiency due to duct
    output.efficiency *= duct_factor;
    output.efficiency = saturate(output.efficiency, 0.0f, 0.95f);

    return output;
}

PropulsionModel::PropulsionOutput PropulsionModel::computeWheel(real_T throttle, real_T angular_velocity) {
    PropulsionOutput output;

    // For wheels, torque is primary output
    // Maximum torque based on motor characteristics
    real_T max_torque = params_.efficiency * 100.0f; // Simplified
    output.torque = throttle * max_torque / params_.gear_ratio;

    // Thrust (traction force) = Torque / radius
    output.thrust = output.torque / (params_.diameter / 2.0f);

    // Power = Torque * angular_velocity
    output.power = output.torque * std::abs(angular_velocity);

    // RPM from angular velocity
    output.rpm = std::abs(angular_velocity) * 60.0f / (2.0f * static_cast<real_T>(M_PI));

    // Efficiency (assume constant for simplicity)
    output.efficiency = params_.efficiency;

    return output;
}

PropulsionModel::PropulsionOutput PropulsionModel::computeTrack(real_T throttle, real_T velocity) {
    // Track system similar to wheels but with continuous contact
    PropulsionOutput output;

    // Tracks have higher starting torque
    real_T max_force = params_.efficiency * 1000.0f; // N
    output.thrust = throttle * max_force;

    // Power based on velocity
    output.power = output.thrust * std::abs(velocity);

    // Torque distributed across track
    output.torque = output.thrust * (params_.diameter / 2.0f);

    // Efficiency (tracks are less efficient due to friction)
    output.efficiency = params_.efficiency * 0.85f;

    return output;
}

PropulsionModel::PropulsionOutput PropulsionModel::computeJet(real_T throttle, real_T velocity) {
    PropulsionOutput output;

    // Simplified jet engine model
    // Thrust approximately constant with speed (simplified)
    real_T max_thrust = 1000.0f * params_.efficiency; // N
    output.thrust = throttle * max_thrust;

    // Specific fuel consumption
    real_T sfc = 0.5f;                            // kg/(N⋅h) simplified
    output.power = output.thrust * sfc * 1000.0f; // W

    // Propulsive efficiency increases with speed
    real_T ideal_velocity = 250.0f; // m/s
    output.efficiency = params_.efficiency * (velocity / (velocity + ideal_velocity));

    return output;
}

real_T PropulsionModel::computeThrustCoefficient(real_T J) const {
    // Simplified Wageningen B-series thrust coefficient
    // Kt = a0 + a1*J + a2*J² + a3*J³
    // These are approximate coefficients for a typical propeller
    real_T a0 = 0.40f;
    real_T a1 = -0.30f;
    real_T a2 = -0.15f;
    real_T a3 = 0.05f;

    real_T Kt = a0 + a1 * J + a2 * J * J + a3 * J * J * J;

    // Ensure Kt is reasonable
    return saturate(Kt, 0.0f, 0.6f);
}

real_T PropulsionModel::computeTorqueCoefficient(real_T J) const {
    // Simplified Wageningen B-series torque coefficient
    // Kq = b0 + b1*J + b2*J² + b3*J³
    real_T b0 = 0.05f;
    real_T b1 = -0.02f;
    real_T b2 = -0.01f;
    real_T b3 = 0.005f;

    real_T Kq = b0 + b1 * J + b2 * J * J + b3 * J * J * J;

    // Ensure Kq is reasonable
    return saturate(Kq, 0.0f, 0.1f);
}

real_T PropulsionModel::computeInducedPower(real_T thrust, real_T velocity) const {
    if (thrust < 0.001f || params_.disk_area < 0.001f) {
        return 0.0f;
    }

    // Momentum theory: P_ind = T^(3/2) / sqrt(2 * ρ * A)
    real_T denominator = std::sqrt(2.0f * params_.air_density * params_.disk_area);
    if (denominator < 0.001f) {
        return 0.0f;
    }

    real_T power = std::pow(thrust, 1.5f) / denominator;

    // Add climb power if moving vertically
    power += thrust * std::abs(velocity);

    return power;
}

real_T PropulsionModel::getMaxThrust() const {
    if (!initialized_) {
        return 0.0f;
    }

    // Compute max thrust by calling compute with max throttle
    PropulsionModel *non_const_this = const_cast<PropulsionModel *>(this);
    PropulsionOutput max_output = non_const_this->compute(1.0f, 0.0f, 0.0f);
    return max_output.thrust;
}

real_T PropulsionModel::getMaxPower() const {
    if (!initialized_) {
        return 0.0f;
    }

    PropulsionModel *non_const_this = const_cast<PropulsionModel *>(this);
    PropulsionOutput max_output = non_const_this->compute(1.0f, 0.0f, 0.0f);
    return max_output.power;
}

void PropulsionModel::setParams(const PropulsionParams &params) { initialize(params); }

void PropulsionModel::setFluidDensity(real_T density) { params_.air_density = density; }

real_T PropulsionModel::saturate(real_T value, real_T min_val, real_T max_val) const {
    return std::max(min_val, std::min(max_val, value));
}

} // namespace autonomylib
} // namespace nervosys
