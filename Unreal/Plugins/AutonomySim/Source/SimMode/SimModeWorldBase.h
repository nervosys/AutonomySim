#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SimModeBase.h"
#include "SimModeWorldBase.generated.h"
#include "api/ApiServerBase.hpp"
#include "api/VehicleSimApiBase.hpp"
#include "common/StateReporterWrapper.hpp"
#include "physics/PhysicsEngineBase.hpp"
#include "physics/PhysicsWorld.hpp"
#include "physics/World.hpp"
#include <memory>
#include <vector>

extern CORE_API uint32 GFrameNumber;

UCLASS()
class AutonomySim_API ASimModeWorldBase : public ASimModeBase {
    GENERATED_BODY()

  public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    virtual void reset() override;
    virtual std::string getDebugReport() override;

    virtual bool isPaused() const override;
    virtual void pause(bool is_paused) override;
    virtual void continueForTime(double seconds) override;
    virtual void continueForFrames(uint32_t frames) override;

    virtual void setWind(const nervosys::autonomylib::Vector3r &wind) const override;
    virtual void setExtForce(const nervosys::autonomylib::Vector3r &ext_force) const override;

  protected:
    void startAsyncUpdator();
    void stopAsyncUpdator();
    virtual void updateDebugReport(nervosys::autonomylib::StateReporterWrapper &debug_reporter) override;

    // should be called by derived class once all api_provider_ is ready to use
    void initializeForPlay();

    // used for adding physics bodies on the fly
    virtual void registerPhysicsBody(nervosys::autonomylib::VehicleSimApiBase *physicsBody) override;

    long long getPhysicsLoopPeriod() const;
    void setPhysicsLoopPeriod(long long period);

  private:
    typedef nervosys::autonomylib::UpdatableObject UpdatableObject;
    typedef nervosys::autonomylib::PhysicsEngineBase PhysicsEngineBase;
    typedef nervosys::autonomylib::ClockFactory ClockFactory;

    // create the physics engine as needed from settings
    std::unique_ptr<PhysicsEngineBase> createPhysicsEngine();

  private:
    std::unique_ptr<nervosys::autonomylib::PhysicsWorld> physics_world_;
    PhysicsEngineBase *physics_engine_;

    /*
    300Hz seems to be minimum for non-aggressive flights
    400Hz is needed for moderately aggressive flights (such as
    high yaw rate with simultaneous back move)
    500Hz is recommended for more aggressive flights
    Lenovo P50 high-end config laptop seems to be topping out at 400Hz.
    HP Z840 desktop high-end config seems to be able to go up to 500Hz.
    To increase freq with limited CPU power, switch Barometer to constant ref mode.
    */
    long long physics_loop_period_ = 3000000LL; // 3ms
};
