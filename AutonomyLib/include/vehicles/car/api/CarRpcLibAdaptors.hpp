// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_CarRpcLibAdaptors_hpp
#define autonomylib_vehicles_CarRpcLibAdaptors_hpp

#include "api/RpcLibAdaptorsBase.hpp"
#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "common/ImageCaptureBase.hpp"
#include "vehicles/car/api/CarApiBase.hpp"

#include "common/utils/WindowsApisCommonPost.hpp"
#include "common/utils/WindowsApisCommonPre.hpp"
#include "rpc/msgpack.hpp"

namespace nervosys {
namespace autonomylib_rpclib {

class CarRpcLibAdaptors : public RpcLibAdaptorsBase {
  public:
    struct CarControls {
        float throttle = 0;
        float steering = 0;
        float brake = 0;
        bool handbrake = false;
        bool is_manual_gear = false;
        int manual_gear = 0;
        bool gear_immediate = true;

        MSGPACK_DEFINE_MAP(throttle, steering, brake, handbrake, is_manual_gear, manual_gear, gear_immediate);

        CarControls() {}

        CarControls(const nervosys::autonomylib::CarApiBase::CarControls &s) {
            throttle = s.throttle;
            steering = s.steering;
            brake = s.brake;
            handbrake = s.handbrake;
            is_manual_gear = s.is_manual_gear;
            manual_gear = s.manual_gear;
            gear_immediate = s.gear_immediate;
        }
        nervosys::autonomylib::CarApiBase::CarControls to() const {
            return nervosys::autonomylib::CarApiBase::CarControls(throttle, steering, brake, handbrake, is_manual_gear,
                                                                  manual_gear, gear_immediate);
        }
    };

    struct CarState {
        float speed;
        int gear;
        float rpm;
        float maxrpm;
        bool handbrake;
        KinematicsState kinematics_estimated;
        uint64_t timestamp;

        MSGPACK_DEFINE_MAP(speed, gear, rpm, maxrpm, handbrake, kinematics_estimated, timestamp);

        CarState() {}

        CarState(const nervosys::autonomylib::CarApiBase::CarState &s) {
            speed = s.speed;
            gear = s.gear;
            rpm = s.rpm;
            maxrpm = s.maxrpm;
            handbrake = s.handbrake;
            timestamp = s.timestamp;
            kinematics_estimated = s.kinematics_estimated;
        }
        nervosys::autonomylib::CarApiBase::CarState to() const {
            return nervosys::autonomylib::CarApiBase::CarState(speed, gear, rpm, maxrpm, handbrake,
                                                               kinematics_estimated.to(), timestamp);
        }
    };
};

} // namespace autonomylib_rpclib
} // namespace nervosys

#endif
