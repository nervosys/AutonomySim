#pragma once

#include "ChaosWheeledVehicleMovementComponent.h"
#include "CoreMinimal.h"

#include "CarPawn.h"
#include "CarPawnApi.h"
#include "PawnEvents.h"
#include "PawnSimApi.h"
#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "physics/Kinematics.hpp"
#include "vehicles/car/CarApiFactory.hpp"
#include "vehicles/car/api/CarApiBase.hpp"

class CarPawnSimApi : public PawnSimApi {
  public:
    typedef nervosys::autonomylib::Utils Utils;
    typedef nervosys::autonomylib::StateReporter StateReporter;
    typedef nervosys::autonomylib::UpdatableObject UpdatableObject;
    typedef nervosys::autonomylib::Pose Pose;

  public:
    virtual void initialize() override;
    virtual ~CarPawnSimApi() = default;

    // VehicleSimApiBase interface
    // implements game interface to update pawn
    CarPawnSimApi(const Params &params, const nervosys::autonomylib::CarApiBase::CarControls &keyboard_controls);

    virtual void update() override;
    virtual void reportState(StateReporter &reporter) override;

    virtual std::string getRecordFileLine(bool is_header_line) const override;

    virtual void updateRenderedState(float dt) override;
    virtual void updateRendering(float dt) override;

    nervosys::autonomylib::CarApiBase *getVehicleApi() const { return vehicle_api_.get(); }

    virtual nervosys::autonomylib::VehicleApiBase *getVehicleApiBase() const override { return vehicle_api_.get(); }

  protected:
    virtual void resetImplementation() override;

  private:
    void updateCarControls();

  private:
    std::unique_ptr<nervosys::autonomylib::CarApiBase> vehicle_api_;
    std::unique_ptr<CarPawnApi> pawn_api_;
    std::vector<std::string> vehicle_api_messages_;

    // storing reference from pawn
    const nervosys::autonomylib::CarApiBase::CarControls &keyboard_controls_;

    nervosys::autonomylib::CarApiBase::CarControls joystick_controls_;
    nervosys::autonomylib::CarApiBase::CarControls current_controls_;
};
