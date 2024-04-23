#ifndef autonomylib_vehicles_simpleflight_firmware_IAxisController_hpp
#define autonomylib_vehicles_simpleflight_firmware_IAxisController_hpp

#include "IBoardClock.hpp"
#include "IGoal.hpp"
#include "IStateEstimator.hpp"
#include "IUpdatable.hpp"
#include <cstdint>

namespace simple_flight {

class IAxisController : public IUpdatable {
  public:
    virtual void initialize(unsigned int axis, const IGoal *goal, const IStateEstimator *state_estimator) = 0;
    virtual TReal getOutput() = 0;

    virtual void reset() override {
        // disable checks for reset/update sequence because
        // this object may get created but not used
        clearResetUpdateAsserts();
        IUpdatable::reset();
    }
};

} // namespace simple_flight

#endif
