#ifndef autonomylib_vehicles_simpleflight_firmware_IController_hpp
#define autonomylib_vehicles_simpleflight_firmware_IController_hpp

#include "IBoardClock.hpp"
#include "IGoal.hpp"
#include "IStateEstimator.hpp"
#include "IUpdatable.hpp"

#include <cstdint>

namespace simple_flight {

class IController : public IUpdatable {

  public:
    virtual void initialize(const IGoal *goal, const IStateEstimator *state_estimator) = 0;
    virtual const Axis4r &getOutput() = 0;
    virtual bool isLastGoalModeAllPassthrough() = 0;
};

} // namespace simple_flight

#endif
