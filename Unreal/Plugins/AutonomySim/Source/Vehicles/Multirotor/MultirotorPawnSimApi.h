#pragma once

#include "CoreMinimal.h"

#include "MultirotorPawnEvents.h"
#include "PawnSimApi.h"
#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "common/common_utils/UniqueValueMap.hpp"
#include "physics//Kinematics.hpp"
#include "vehicles/multirotor/MultirotorParams.hpp"
#include "vehicles/multirotor/MultirotorPhysicsBody.hpp"
#include <future>

class MultirotorPawnSimApi : public PawnSimApi {
  public:
    typedef nervosys::autonomylib::real_T real_T;
    typedef nervosys::autonomylib::Utils Utils;
    typedef nervosys::autonomylib::MultirotorPhysicsBody Multirotor;
    typedef nervosys::autonomylib::StateReporter StateReporter;
    typedef nervosys::autonomylib::UpdatableObject UpdatableObject;
    typedef nervosys::autonomylib::Pose Pose;

    typedef MultirotorPawnEvents::RotorActuatorInfo RotorActuatorInfo;

  public:
    virtual void initialize() override;

    virtual ~MultirotorPawnSimApi() = default;

    // VehicleSimApiBase interface
    // implements game interface to update pawn
    MultirotorPawnSimApi(const Params &params);
    virtual void updateRenderedState(float dt) override;
    virtual void updateRendering(float dt) override;

    // PhysicsBody interface
    // this just wrapped around Multirotor physics body
    virtual void resetImplementation() override;
    virtual void update() override;
    virtual void reportState(StateReporter &reporter) override;
    virtual UpdatableObject *getPhysicsBody() override;

    virtual void setPose(const Pose &pose, bool ignore_collision) override;
    virtual void setKinematics(const Kinematics::State &state, bool ignore_collision) override;
    virtual void pawnTick(float dt) override;

    nervosys::autonomylib::MultirotorApiBase *getVehicleApi() const { return vehicle_api_.get(); }

    virtual nervosys::autonomylib::VehicleApiBase *getVehicleApiBase() const override { return vehicle_api_.get(); }

  private:
    std::unique_ptr<nervosys::autonomylib::MultirotorApiBase> vehicle_api_;
    std::unique_ptr<nervosys::autonomylib::MultirotorParams> vehicle_params_;

    std::unique_ptr<Multirotor> multirotor_physics_body_;
    unsigned int rotor_count_;
    std::vector<RotorActuatorInfo> rotor_actuator_info_;

    // show info on collision response from physics engine
    CollisionResponse collision_response;

    MultirotorPawnEvents *pawn_events_;

    // when pose needs to set from non-physics thread, we set it as pending
    bool pending_pose_collisions_;
    enum class PendingPoseStatus { NonePending, RenderPending } pending_pose_status_;
    Pose pending_phys_pose_; // force new pose through API

    // reset must happen while World is locked so its async task initiated from API thread
    bool reset_pending_;
    bool did_reset_;
    std::packaged_task<void()> reset_task_;

    Pose last_phys_pose_; // for trace lines showing vehicle path
    std::vector<std::string> vehicle_api_messages_;
    RotorStates rotor_states_;
};
