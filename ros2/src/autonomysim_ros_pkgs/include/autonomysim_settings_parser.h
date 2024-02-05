#include "common/common_utils/StrictMode.hpp"
STRICT_MODE_OFF
#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK
#include "rpc/rpc_error.h"
STRICT_MODE_ON

#include "common/AutonomySimSettings.hpp"
#include "common/common_utils/FileSystem.hpp"
#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"
#include <chrono>
#include <iostream>

// a minimal AutonomySim settings parser, adapted from Unreal/Plugins/AutonomySim/SimHUD/SimHUD.h
class AutonomySimSettingsParser {
  public:
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;

  public:
    AutonomySimSettingsParser(const std::string &host_ip);
    ~AutonomySimSettingsParser() = default;

    bool success();

  private:
    std::string getSimMode();
    bool getSettingsText(std::string &settings_text) const;
    bool initializeSettings();

    bool success_;
    std::string settings_text_;
    std::string host_ip_;
};