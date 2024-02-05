#ifndef autonomylib_unittests_SettingsTest_hpp
#define autonomylib_unittests_SettingsTest_hpp

#include "TestBase.hpp"
#include "common/Settings.hpp"

namespace nervosys {
namespace autonomylib {

class SettingsTest : public TestBase {
  public:
    virtual void run() override {
        Settings &settings = Settings::loadJSonFile("settings.json");
        unused(settings);
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif