// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_CarRpcLibServer_hpp
#define autonomylib_vehicles_CarRpcLibServer_hpp

#ifndef AUTONOMYLIB_NO_RPC

#include "api/RpcLibServerBase.hpp"
#include "common/Common.hpp"
#include "vehicles/car/api/CarApiBase.hpp"

#include <functional>

namespace nervosys {
namespace autonomylib {

class CarRpcLibServer : public RpcLibServerBase {

  protected:
    virtual CarApiBase *getVehicleApi(const std::string &vehicle_name) override {
        return static_cast<CarApiBase *>(RpcLibServerBase::getVehicleApi(vehicle_name));
    }

  public:
    CarRpcLibServer(ApiProvider *api_provider, string server_address, uint16_t port = RpcLibPort);
    virtual ~CarRpcLibServer();
};

#endif

} // namespace autonomylib
} // namespace nervosys

#endif