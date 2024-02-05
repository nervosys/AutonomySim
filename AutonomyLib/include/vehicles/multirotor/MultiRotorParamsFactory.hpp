// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_MultirotorParamsFactory_hpp
#define autonomylib_vehicles_MultirotorParamsFactory_hpp

#include "vehicles/multirotor/firmwares/arducopter/ArduCopterParams.hpp"
#include "vehicles/multirotor/firmwares/mavlink/ArduCopterSoloParams.hpp"
#include "vehicles/multirotor/firmwares/mavlink/MavLinkMultirotorApi.hpp"
#include "vehicles/multirotor/firmwares/mavlink/Px4MultirotorParams.hpp"
#include "vehicles/multirotor/firmwares/simple_flight/SimpleFlightQuadXParams.hpp"

namespace nervosys {
namespace autonomylib {

class MultirotorParamsFactory {
  public:
    static std::unique_ptr<MultirotorParams> createConfig(const AutonomySimSettings::VehicleSetting *vehicle_setting,
                                                          std::shared_ptr<const SensorFactory> sensor_factory) {
        std::unique_ptr<MultirotorParams> config;

        if (vehicle_setting->vehicle_type == AutonomySimSettings::kVehicleTypePX4) {
            config.reset(new Px4MultirotorParams(
                *static_cast<const AutonomySimSettings::MavLinkVehicleSetting *>(vehicle_setting), sensor_factory));
        } else if (vehicle_setting->vehicle_type == AutonomySimSettings::kVehicleTypeArduCopterSolo) {
            config.reset(new ArduCopterSoloParams(
                *static_cast<const AutonomySimSettings::MavLinkVehicleSetting *>(vehicle_setting), sensor_factory));
        } else if (vehicle_setting->vehicle_type == AutonomySimSettings::kVehicleTypeArduCopter) {
            config.reset(new ArduCopterParams(
                *static_cast<const AutonomySimSettings::MavLinkVehicleSetting *>(vehicle_setting), sensor_factory));
        } else if (vehicle_setting->vehicle_type == "" || // default config
                   vehicle_setting->vehicle_type == AutonomySimSettings::kVehicleTypeSimpleFlight) {
            config.reset(new SimpleFlightQuadXParams(vehicle_setting, sensor_factory));
        } else {
            throw std::runtime_error(
                Utils::stringf("Cannot create vehicle config because vehicle name '%s' is not recognized",
                               vehicle_setting->vehicle_name.c_str()));
        }

        config->initialize(vehicle_setting);

        return config;
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif
