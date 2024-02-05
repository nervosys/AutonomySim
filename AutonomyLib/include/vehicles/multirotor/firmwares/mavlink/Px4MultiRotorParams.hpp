// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_Px4MultirotorParams_hpp
#define autonomylib_vehicles_Px4MultirotorParams_hpp

#include "common/AutonomySimSettings.hpp"
#include "sensors/SensorFactory.hpp"
#include "vehicles/multirotor/MultirotorParams.hpp"
#include "vehicles/multirotor/firmwares/mavlink/MavLinkMultirotorApi.hpp"

namespace nervosys {
namespace autonomylib {

class Px4MultirotorParams : public MultirotorParams {
  public:
    Px4MultirotorParams(const AutonomySimSettings::MavLinkVehicleSetting &vehicle_setting,
                        std::shared_ptr<const SensorFactory> sensor_factory)
        : sensor_factory_(sensor_factory) {
        connection_info_ = getConnectionInfo(vehicle_setting);
    }

    virtual ~Px4MultirotorParams() = default;

    virtual std::unique_ptr<MultirotorApiBase> createMultirotorApi() override {
        unique_ptr<MultirotorApiBase> api(new MavLinkMultirotorApi());
        auto api_ptr = static_cast<MavLinkMultirotorApi *>(api.get());
        api_ptr->initialize(connection_info_, &getSensors(), true);

        return api;
    }

    virtual void setupParams() override {
        auto &params = getParams();

        if (connection_info_.model == "Blacksheep") {
            setupFrameBlacksheep(params);
        } else if (connection_info_.model == "Flamewheel") {
            setupFrameFlamewheel(params);
        } else if (connection_info_.model == "FlamewheelFLA") {
            setupFrameFlamewheelFLA(params);
        } else if (connection_info_.model == "Hexacopter") {
            setupFrameGenericHex(params);
        } else if (connection_info_.model == "Octocopter") {
            setupFrameGenericOcto(params);
        } else // Generic
            setupFrameGenericQuad(params);
    }

  protected:
    virtual const SensorFactory *getSensorFactory() const override { return sensor_factory_.get(); }

  private:
    static const AutonomySimSettings::MavLinkConnectionInfo &
    getConnectionInfo(const AutonomySimSettings::MavLinkVehicleSetting &vehicle_setting) {
        return vehicle_setting.connection_info;
    }

  private:
    AutonomySimSettings::MavLinkConnectionInfo connection_info_;
    std::shared_ptr<const SensorFactory> sensor_factory_;
};

} // namespace autonomylib
} // namespace nervosys

#endif
