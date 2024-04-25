// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_MultirotorRpcLibServer_hpp
#define autonomylib_vehicles_MultirotorRpcLibServer_hpp

#include "api/RpcLibServerBase.hpp"
#include "common/Common.hpp"
#include "vehicles/multirotor/api/MultirotorApiBase.hpp"
#include <functional>

namespace nervosys {
namespace autonomylib {

class MultirotorRpcLibServer : public RpcLibServerBase {

  protected:
    virtual MultirotorApiBase *getVehicleApi(const std::string &vehicle_name) override {
        return static_cast<MultirotorApiBase *>(RpcLibServerBase::getVehicleApi(vehicle_name));
    }

  public:
    MultirotorRpcLibServer(ApiProvider *api_provider, string server_address, uint16_t port = RpcLibPort);
    virtual ~MultirotorRpcLibServer();
};

} // namespace autonomylib
} // namespace nervosys

#endif
