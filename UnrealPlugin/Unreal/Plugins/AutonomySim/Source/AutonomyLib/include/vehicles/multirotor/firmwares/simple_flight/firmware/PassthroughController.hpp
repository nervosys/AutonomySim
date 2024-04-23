#ifndef autonomylib_vehicles_simpleflight_firmware_PassthroughController_hpp
#define autonomylib_vehicles_simpleflight_firmware_PassthroughController_hpp

#include "Params.hpp"
#include "common/utils
#include "interfaces/CommonStructs.hpp"
#include "interfaces/IAxisController.hpp"
#include "interfaces/IBoardClock.hpp"
#include <memory>

namespace simple_flight {

class PassthroughController : public IAxisController {
  public:
    virtual void initialize(unsigned int axis, const IGoal *goal, const IStateEstimator *state_estimator) override {
        axis_ = axis;
        goal_ = goal;
        unused(state_estimator);
    }

    virtual void reset() override {
        IAxisController::reset();
        output_ = TReal();
    }

    virtual void update() override {
        IAxisController::update();
        output_ = goal_->getGoalValue()[axis_];
    }

    virtual TReal getOutput() override { return output_; }

  private:
    unsigned int axis_;
    const IGoal *goal_;
    TReal output_;
};

} // namespace simple_flight

#endif
