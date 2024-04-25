// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_sensors_BarometerBase_hpp
#define autonomylib_sensors_BarometerBase_hpp

#include "sensors/SensorBase.hpp"

namespace nervosys {
namespace autonomylib {

class BarometerBase : public SensorBase {

  private:
    Output output_;

  protected:
    void setOutput(const Output &output) { output_ = output; }

  public:
    BarometerBase(const std::string &sensor_name = "") : SensorBase(sensor_name) {}

    // types
    struct Output { // same fields as ROS message
        TTimePoint time_stamp;
        real_T altitude; // meters
        real_T pressure; // Pascal
        real_T qnh;
    };

    virtual void reportState(StateReporter &reporter) override {
        // call base
        UpdatableObject::reportState(reporter);

        reporter.writeValue("Baro-Alt", output_.altitude);
        reporter.writeValue("Baro-Prs", output_.pressure);
    }

    const Output &getOutput() const { return output_; }
};

} // namespace autonomylib
} // namespace nervosys

#endif
