// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_ArduCopterParams_hpp
#define autonomylib_vehicles_ArduCopterParams_hpp

#include "common/AutonomySimSettings.hpp"
#include "sensors/SensorFactory.hpp"
#include "vehicles/multirotor/MultirotorParams.hpp"
#include "vehicles/multirotor/firmwares/arducopter/ArduCopterApi.hpp"

namespace nervosys {
namespace autonomylib {

class ArduCopterParams : public MultirotorParams {

  private:
    AutonomySimSettings::MavLinkConnectionInfo connection_info_;
    std::shared_ptr<const SensorFactory> sensor_factory_;

  protected:
    virtual void setupParams() override {
        auto &params = getParams();

        // Use connection_info_.model for the model name, see Px4MultirotorParams for example

        // Only Generic for now
        setupFrameGenericQuad(params);
    }

    virtual const SensorFactory *getSensorFactory() const override { return sensor_factory_.get(); }

    static const AutonomySimSettings::MavLinkConnectionInfo &
    getConnectionInfo(const AutonomySimSettings::MavLinkVehicleSetting &vehicle_setting) {
        return vehicle_setting.connection_info;
    }

  public:
    ArduCopterParams(const AutonomySimSettings::MavLinkVehicleSetting &vehicle_setting,
                     std::shared_ptr<const SensorFactory> sensor_factory)
        : sensor_factory_(sensor_factory) {
        connection_info_ = getConnectionInfo(vehicle_setting);
    }

    virtual ~ArduCopterParams() = default;

    virtual std::unique_ptr<MultirotorApiBase> createMultirotorApi() override {
        return std::unique_ptr<MultirotorApiBase>(new ArduCopterApi(this, connection_info_));
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif
