#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "PawnEvents.h"
#include "common/Common.hpp"
#include "common/utils/Signal.hpp"

class MultirotorPawnEvents : public PawnEvents {
  public: // types
    typedef nervosys::autonomylib::real_T real_T;
    struct RotorActuatorInfo {
        real_T rotor_speed = 0;
        int rotor_direction = 0;
        real_T rotor_thrust = 0;
        real_T rotor_control_filtered = 0;
    };

    typedef common_utils::Signal<const std::vector<RotorActuatorInfo> &> ActuatorsSignal;

  public:
    ActuatorsSignal &getActuatorSignal();

  private:
    ActuatorsSignal actuator_signal_;
};
