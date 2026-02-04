// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_sensors_MagnetometerBase_hpp
#define autonomylib_sensors_MagnetometerBase_hpp

#include "sensors/SensorBase.hpp"

namespace nervosys {
namespace autonomylib {

class MagnetometerBase : public SensorBase {

  public:
    struct Output { // same fields as ROS message
        TTimePoint time_stamp;
        Vector3r magnetic_field_body;             // in Gauss
        vector<real_T> magnetic_field_covariance; // 9 elements 3x3 matrix
    };

    MagnetometerBase(const std::string &sensor_name = "") : SensorBase(sensor_name) {}

    virtual void reportState(StateReporter &reporter) override {
        // call base
        UpdatableObject::reportState(reporter);

        reporter.writeValue("Mag-Vec", output_.magnetic_field_body);
    }

    const Output &getOutput() const { return output_; }

  protected:
    void setOutput(const Output &output) { output_ = output; }

  private:
    Output output_;
};

} // namespace autonomylib
} // namespace nervosys
#endif
