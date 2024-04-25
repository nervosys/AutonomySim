// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_sensors_LidarSimple_hpp
#define autonomylib_sensors_LidarSimple_hpp

#include "LidarBase.hpp"
#include "LidarSimpleParams.hpp"
#include "common/Common.hpp"
#include "common/DelayLine.hpp"
#include "common/FrequencyLimiter.hpp"

#include <random>

namespace nervosys {
namespace autonomylib {

class LidarSimple : public LidarBase {

  private:
    LidarSimpleParams params_;
    vector<real_T> point_cloud_;
    vector<int> segmentation_cloud_;

    FrequencyLimiter freq_limiter_;
    TTimePoint last_time_;

    void updateOutput() {
        TTimeDelta delta_time = clock()->updateSince(last_time_);

        point_cloud_.clear();

        const GroundTruth &ground_truth = getGroundTruth();

        // calculate the pose before obtaining the point-cloud. Before/after is a bit arbitrary
        // decision here. If the pose can change while obtaining the point-cloud (could happen for drones)
        // then the pose won't be very accurate either way.
        //
        // TODO: Seems like pose is in vehicle inertial-frame (NOT in Global NED frame).
        //    That could be a bit unintuitive but seems consistent with the position/orientation returned as part of
        //    ImageResponse for cameras and pose returned by getCameraInfo API.
        //    Do we need to convert pose to Global NED frame before returning to clients?
        Pose lidar_pose = params_.relative_pose + ground_truth.kinematics->pose;
        getPointCloud(params_.relative_pose,         // relative lidar pose
                      ground_truth.kinematics->pose, // relative vehicle pose
                      delta_time, point_cloud_, segmentation_cloud_);

        LidarData output;
        output.point_cloud = point_cloud_;
        output.time_stamp = clock()->nowNanos();
        output.pose = lidar_pose;
        output.segmentation = segmentation_cloud_;

        last_time_ = output.time_stamp;

        setOutput(output);
    }

  protected:
    virtual void getPointCloud(const Pose &lidar_pose, const Pose &vehicle_pose, TTimeDelta delta_time,
                               vector<real_T> &point_cloud, vector<int> &segmentation_cloud) = 0;

  public:
    LidarSimple(const AutonomySimSettings::LidarSetting &setting = AutonomySimSettings::LidarSetting())
        : LidarBase(setting.sensor_name) {
        // initialize params
        params_.initializeFromSettings(setting);

        // initialize frequency limiter
        freq_limiter_.initialize(params_.update_frequency, params_.startup_delay);
    }

    //*** Start: UpdatableState implementation ***//
    virtual void resetImplementation() override {
        freq_limiter_.reset();
        last_time_ = clock()->nowNanos();

        updateOutput();
    }

    virtual void update() override {
        LidarBase::update();

        freq_limiter_.update();

        if (freq_limiter_.isWaitComplete()) {
            updateOutput();
        }
    }

    virtual void reportState(StateReporter &reporter) override {
        // call base
        LidarBase::reportState(reporter);

        reporter.writeValue("Lidar-NumChannels", params_.number_of_channels);
        reporter.writeValue("Lidar-Range", params_.range);
        reporter.writeValue("Lidar-FOV-Upper", params_.vertical_FOV_upper);
        reporter.writeValue("Lidar-FOV-Lower", params_.vertical_FOV_lower);
    }
    //*** End: UpdatableState implementation ***//

    virtual ~LidarSimple() = default;

    const LidarSimpleParams &getParams() const { return params_; }
};

} // namespace autonomylib
} // namespace nervosys

#endif
