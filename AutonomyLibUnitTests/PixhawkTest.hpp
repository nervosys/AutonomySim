#ifndef autonomylib_unittests_PixhawkTest_hpp
#define autonomylib_unittests_PixhawkTest_hpp

#include "TestBase.hpp"
#include "vehicles/multirotor/MultirotorParamsFactory.hpp"

namespace nervosys {
namespace autonomylib {

class PixhawkTest : public TestBase {

  public:
    virtual void run() override {
        // Test PX4 based drones
        auto pixhawk = MultirotorParamsFactory::createConfig(
            AutonomySimSettings::singleton().getVehicleSetting("Pixhawk"), std::make_shared<SensorFactory>());
        auto api = pixhawk->createMultirotorApi();
        testAssert(api != nullptr, "Couldn't get pixhawk controller");

        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } catch (std::domain_error &ex) {
            std::cout << ex.what() << std::endl;
        }
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif