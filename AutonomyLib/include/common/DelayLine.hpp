// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_common_DelayLine_hpp
#define autonomylib_common_DelayLine_hpp

#include "Common.hpp"
#include "UpdatableObject.hpp"

#include <list>

namespace nervosys {
namespace autonomylib {

template <typename T> class DelayLine : public UpdatableObject {

  private:
    template <typename TItem> using list = std::list<TItem>;

    list<T> values_;
    list<TTimePoint> times_;
    TTimeDelta delay_;

    T last_value_;
    TTimePoint last_time_;

  public:
    DelayLine() {}
    DelayLine(TTimeDelta delay) { // in seconds
        initialize(delay);
    }
    void initialize(TTimeDelta delay) { // in seconds
        setDelay(delay);
    }
    void setDelay(TTimeDelta delay) { delay_ = delay; }
    double getDelay() const { return delay_; }

    //*** Start: UpdatableState implementation ***//
    virtual void resetImplementation() override {
        values_.clear();
        times_.clear();
        last_time_ = 0;
        last_value_ = T();
    }

    virtual void update() override {
        UpdatableObject::update();

        if (!times_.empty() && ClockBase::elapsedBetween(clock()->nowNanos(), times_.front()) >= delay_) {

            last_value_ = values_.front();
            last_time_ = times_.front();

            times_.pop_front();
            values_.pop_front();
        }
    }
    //*** End: UpdatableState implementation ***//

    T getOutput() const { return last_value_; }
    double getOutputTime() const { return last_time_; }

    void push_back(const T &val, TTimePoint time_offset = 0) {
        values_.push_back(val);
        times_.push_back(clock()->nowNanos() + time_offset);
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif
