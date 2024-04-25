
#ifndef autonomylib_unittests_SimpleFlightTest_hpp
#define autonomylib_unittests_SimpleFlightTest_hpp

#include "TestBase.hpp"
#include "common/SteppableClock.hpp"
#include "physics/FastPhysicsEngine.hpp"
#include "physics/PhysicsWorld.hpp"
#include "vehicles/multirotor/MultirotorParamsFactory.hpp"
#include "vehicles/multirotor/MultirotorPhysicsBody.hpp"
#include "vehicles/multirotor/api/MultirotorApiBase.hpp"

namespace nervosys {
namespace autonomylib {

class SimpleFlightTest : public TestBase {

  private:
    std::vector<std::string> messages_;

  public:
    virtual void run() override {

        auto clock = std::make_shared<SteppableClock>(3E-3f);
        ClockFactory::get(clock);
        SensorFactory sensor_factory;

        std::unique_ptr<MultirotorParams> params = MultirotorParamsFactory::createConfig(
            AutonomySimSettings::singleton().getVehicleSetting("SimpleFlight"), std::make_shared<SensorFactory>());
        auto api = params->createMultirotorApi();

        std::unique_ptr<nervosys::autonomylib::Kinematics> kinematics;
        std::unique_ptr<nervosys::autonomylib::Environment> environment;
        Kinematics::State initial_kinematic_state = Kinematics::State::zero();
        ;
        initial_kinematic_state.pose = Pose();
        kinematics.reset(new Kinematics(initial_kinematic_state));

        Environment::State initial_environment;
        initial_environment.position = initial_kinematic_state.pose.position;
        initial_environment.geo_point = GeoPoint();
        environment.reset(new Environment(initial_environment));

        MultirotorPhysicsBody vehicle(params.get(), api.get(), kinematics.get(), environment.get());

        std::vector<UpdatableObject *> vehicles = {&vehicle};
        std::unique_ptr<PhysicsEngineBase> physics_engine(new FastPhysicsEngine());
        PhysicsWorld physics_world(std::move(physics_engine), vehicles,
                                   static_cast<uint64_t>(clock->getStepSize() * 1E9));

        testAssert(api != nullptr, "api was null");
        std::string message;
        testAssert(api->isReady(message), message);

        clock->sleep_for(0.04f);

        Utils::getSetMinLogLevel(true, 100);

        api->enableApiControl(true);
        api->armDisarm(true);
        api->takeoff(10);

        clock->sleep_for(2.0f);

        Utils::getSetMinLogLevel(true);

        api->moveToPosition(-5, -5, -5, 5, 1E3, DrivetrainType::MaxDegreeOfFreedom, YawMode(true, 0), -1, 0);

        clock->sleep_for(2.0f);

        while (true) {
            clock->sleep_for(0.1f);
            api->getStatusMessages(messages_);
            for (const auto &status_message : messages_) {
                std::cout << status_message << std::endl;
            }
            messages_.clear();
        }
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif