#pragma once

#include "CarPawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "physics/Kinematics.hpp"
#include "vehicles/car/api/CarApiBase.hpp"

class CarPawnApi {
  public:
    typedef nervosys::autonomylib::ImageCaptureBase ImageCaptureBase;

    CarPawnApi(ACarPawn *pawn, const nervosys::autonomylib::Kinematics::State *pawn_kinematics,
               nervosys::autonomylib::CarApiBase *vehicle_api);

    void updateMovement(const nervosys::autonomylib::CarApiBase::CarControls &controls);

    nervosys::autonomylib::CarApiBase::CarState getCarState() const;

    void reset();
    void update();

    virtual ~CarPawnApi();

  private:
    UChaosWheeledVehicleMovementComponent *movement_;
    nervosys::autonomylib::CarApiBase::CarControls last_controls_;
    ACarPawn *pawn_;
    const nervosys::autonomylib::Kinematics::State *pawn_kinematics_;
    nervosys::autonomylib::CarApiBase *vehicle_api_;
};
