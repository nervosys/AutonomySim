// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_sensors_DistanceBase_hpp
#define autonomylib_sensors_DistanceBase_hpp

#include "sensors/SensorBase.hpp"

namespace nervosys {
namespace autonomylib {

class DistanceBase : public SensorBase {

  private:
    DistanceSensorData output_;

  protected:
    void setOutput(const DistanceSensorData &output) { output_ = output; }

  public:
    DistanceBase(const std::string &sensor_name = "") : SensorBase(sensor_name) {}

    virtual void reportState(StateReporter &reporter) override {
        // call base
        UpdatableObject::reportState(reporter);

        reporter.writeValue("Dist-Curr", output_.distance);
    }

    const DistanceSensorData &getOutput() const { return output_; }
};

} // namespace autonomylib
} // namespace nervosys

#endif
