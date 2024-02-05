// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "GameFramework/Actor.h"
#include "NedTransform.h"
#include "common/Common.hpp"
#include "sensors/lidar/LidarSimple.hpp"

// UnrealLidarSensor implementation that uses Ray Tracing in Unreal.
// The implementation uses a model similar to CARLA Lidar implementation.
// Thanks to CARLA folks for this.
class UnrealLidarSensor : public nervosys::autonomylib::LidarSimple {
  public:
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;

  public:
    UnrealLidarSensor(const AutonomySimSettings::LidarSetting &setting, AActor *actor,
                      const NedTransform *ned_transform);

  protected:
    virtual void getPointCloud(const nervosys::autonomylib::Pose &lidar_pose, const nervosys::autonomylib::Pose &vehicle_pose,
                               nervosys::autonomylib::TTimeDelta delta_time,
                               nervosys::autonomylib::vector<nervosys::autonomylib::real_T> &point_cloud,
                               nervosys::autonomylib::vector<int> &segmentation_cloud) override;

  private:
    using Vector3r = nervosys::autonomylib::Vector3r;
    using VectorMath = nervosys::autonomylib::VectorMath;

    void createLasers();
    bool shootLaser(const nervosys::autonomylib::Pose &lidar_pose, const nervosys::autonomylib::Pose &vehicle_pose,
                    const float horizontal_angle, const float vertical_angle,
                    const nervosys::autonomylib::LidarSimpleParams &params, Vector3r &point, int &segmentationID);

  private:
    AActor *actor_;
    const NedTransform *ned_transform_;

    nervosys::autonomylib::vector<nervosys::autonomylib::real_T> laser_angles_;
    float current_horizontal_angle_ = 0.0f;
};