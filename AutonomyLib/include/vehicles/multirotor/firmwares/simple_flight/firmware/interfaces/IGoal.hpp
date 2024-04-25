#ifndef autonomylib_vehicles_simpleflight_firmware_IGoal_hpp
#define autonomylib_vehicles_simpleflight_firmware_IGoal_hpp

#include "CommonStructs.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IGoal {

  public:
    virtual const Axis4r &getGoalValue() const = 0;
    virtual const GoalMode &getGoalMode() const = 0;

    virtual ~IGoal() = default;
};

} // namespace simple_flight

#endif
