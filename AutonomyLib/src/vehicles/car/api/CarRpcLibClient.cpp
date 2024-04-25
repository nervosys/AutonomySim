// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// in header only mode, control library is not available
#ifndef AUTONOMYLIB_HEADER_ONLY
// RPC code requires C++14. If build system like Unreal doesn't support it then use compiled binaries
#ifndef AUTONOMYLIB_NO_RPC
// if using Unreal Build system then include precompiled header file first

#include "vehicles/car/api/CarRpcLibClient.hpp"

#include "common/ClockFactory.hpp"
#include "common/Common.hpp"

#include <thread>
STRICT_MODE_OFF

#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK

#ifdef nil
#undef nil
#endif // nil

#include "common/utils/WindowsApisCommonPre.hpp"
#undef FLOAT
#undef check
#include "rpc/client.h"
// TODO: HACK: UE4 defines macro with stupid names like "check" that conflicts with msgpack library
#ifndef check
#define check(expr) (static_cast<void>((expr)))
#endif
#include "common/utils/WindowsApisCommonPost.hpp"
#include "vehicles/car/api/CarRpcLibAdaptors.hpp"

STRICT_MODE_ON
#ifdef _MSC_VER
__pragma(warning(disable : 4239))
#endif

    namespace nervosys {
    namespace autonomylib {

    typedef nervosys::autonomylib_rpclib::CarRpcLibAdaptors CarRpcLibAdaptors;

    CarRpcLibClient::CarRpcLibClient(const string &ip_address, uint16_t port, float timeout_sec)
        : RpcLibClientBase(ip_address, port, timeout_sec) {}

    CarRpcLibClient::~CarRpcLibClient() {}

    void CarRpcLibClient::setCarControls(const CarApiBase::CarControls &controls, const std::string &vehicle_name) {
        static_cast<rpc::client *>(getClient())
            ->call("setCarControls", CarRpcLibAdaptors::CarControls(controls), vehicle_name);
    }

    CarApiBase::CarState CarRpcLibClient::getCarState(const std::string &vehicle_name) {
        return static_cast<rpc::client *>(getClient())
            ->call("getCarState", vehicle_name)
            .as<CarRpcLibAdaptors::CarState>()
            .to();
    }

    CarApiBase::CarControls CarRpcLibClient::getCarControls(const std::string &vehicle_name) {
        return static_cast<rpc::client *>(getClient())
            ->call("getCarControls", vehicle_name)
            .as<CarRpcLibAdaptors::CarControls>()
            .to();
    }

    } // namespace autonomylib
} // namespace nervosys

#endif
#endif
