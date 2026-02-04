// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_sensors_LidarBase_hpp
#define autonomylib_sensors_LidarBase_hpp

#include "sensors/SensorBase.hpp"

namespace nervosys {
namespace autonomylib {

class LidarBase : public SensorBase {

  private:
    LidarData output_;

  protected:
    void setOutput(const LidarData &output) { output_ = output; }

  public:
    LidarBase(const std::string &sensor_name = "") : SensorBase(sensor_name) {}

    virtual void reportState(StateReporter &reporter) override {
        // call base
        UpdatableObject::reportState(reporter);

        reporter.writeValue("Lidar-Timestamp", output_.time_stamp);
        reporter.writeValue("Lidar-NumPoints", static_cast<int>(output_.point_cloud.size() / 3));
    }

    const LidarData &getOutput() const { return output_; }
};

} // namespace autonomylib
} // namespace nervosys

#endif
