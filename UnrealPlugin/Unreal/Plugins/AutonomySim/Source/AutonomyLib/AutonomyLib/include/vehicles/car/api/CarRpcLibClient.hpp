// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_CarRpcLibClient_hpp
#define autonomylib_vehicles_CarRpcLibClient_hpp

#include "api/RpcLibClientBase.hpp"
#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "common/ImageCaptureBase.hpp"
#include "vehicles/car/api/CarApiBase.hpp"

#include <functional>

namespace nervosys {
namespace autonomylib {

class CarRpcLibClient : public RpcLibClientBase {

  public:
    CarRpcLibClient(const string &ip_address = "localhost", uint16_t port = RpcLibPort, float timeout_sec = 60);

    void setCarControls(const CarApiBase::CarControls &controls, const std::string &vehicle_name = "");
    CarApiBase::CarState getCarState(const std::string &vehicle_name = "");
    CarApiBase::CarControls getCarControls(const std::string &vehicle_name = "");
    virtual ~CarRpcLibClient(); // required for pimpl
};

} // namespace autonomylib
} // namespace nervosys

#endif
