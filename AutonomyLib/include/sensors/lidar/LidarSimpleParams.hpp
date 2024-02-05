// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_sensors_LidarSimpleParams_hpp
#define autonomylib_sensors_LidarSimpleParams_hpp

#include "common/AutonomySimSettings.hpp"
#include "common/Common.hpp"

namespace nervosys {
namespace autonomylib {

struct LidarSimpleParams {

    // Velodyne VLP-16 Puck config
    // https://velodynelidar.com/vlp-16.html

    // default settings
    // TODO: enable reading of these params from AutonomySim settings

    uint number_of_channels = 16;
    real_T range = 10000.0f / 100; // meters
    uint points_per_second = 100000;
    uint horizontal_rotation_frequency = 10; // rotations/sec
    real_T horizontal_FOV_start = 0;
    real_T horizontal_FOV_end = 359;
    real_T vertical_FOV_upper = -15; // drones -15, car +10
    real_T vertical_FOV_lower = -45; // drones -45, car -10

    Pose relative_pose{
        Vector3r(0, 0, -1),     // position - a little above vehicle (especially for cars) or Vector3r::Zero()
        Quaternionr::Identity() // orientation - by default Quaternionr(1, 0, 0, 0)
    };

    bool draw_debug_points = false;
    AutonomySimSettings::LidarSetting::DataFrame data_frame;
    bool external_controller = true;
    real_T update_frequency = 10; // Hz
    real_T startup_delay = 0;     // sec

    void initializeFromSettings(const AutonomySimSettings::LidarSetting &settings) {
        std::string simmode_name = AutonomySimSettings::singleton().simmode_name;

        const auto &settings_json = settings.settings;
        number_of_channels = settings_json.getInt("NumberOfChannels", number_of_channels);
        range = settings_json.getFloat("Range", range);
        points_per_second = settings_json.getInt("PointsPerSecond", points_per_second);
        horizontal_rotation_frequency = settings_json.getInt("RotationsPerSecond", horizontal_rotation_frequency);
        draw_debug_points = settings_json.getBool("DrawDebugPoints", draw_debug_points);
        std::string frame = settings_json.getString("DataFrame", AutonomySimSettings::kVehicleInertialFrame);

        if (frame == AutonomySimSettings::kVehicleInertialFrame) {
            data_frame = AutonomySimSettings::LidarSetting::DataFrame::VehicleInertialFrame;
        } else if (frame == AutonomySimSettings::kSensorLocalFrame) {
            data_frame = AutonomySimSettings::LidarSetting::DataFrame::SensorLocalFrame;
        } else {
            throw std::runtime_error("Unknown requested data frame");
        }
        
        external_controller = settings_json.getBool("ExternalController", external_controller);
        vertical_FOV_upper = settings_json.getFloat("VerticalFOVUpper", Utils::nan<float>());

        // By default, for multirotors the lidars FOV point downwards;
        // for cars, the lidars FOV is more forward facing.
        if (std::isnan(vertical_FOV_upper)) {
            if (simmode_name == AutonomySimSettings::kSimModeTypeMultirotor) {
                vertical_FOV_upper = -15;
            } else {
                vertical_FOV_upper = +10;
            }
        }

        vertical_FOV_lower = settings_json.getFloat("VerticalFOVLower", Utils::nan<float>());
        if (std::isnan(vertical_FOV_lower)) {
            if (simmode_name == AutonomySimSettings::kSimModeTypeMultirotor) {
                vertical_FOV_lower = -45;
            } else {
                vertical_FOV_lower = -10;
            }
        }

        horizontal_FOV_start = settings_json.getFloat("HorizontalFOVStart", horizontal_FOV_start);
        horizontal_FOV_end = settings_json.getFloat("HorizontalFOVEnd", horizontal_FOV_end);

        relative_pose.position = AutonomySimSettings::createVectorSetting(settings_json, VectorMath::nanVector());
        auto rotation =
            AutonomySimSettings::createRotationSetting(settings_json, AutonomySimSettings::Rotation::nanRotation());

        if (std::isnan(relative_pose.position.x())) {
            relative_pose.position.x() = 0;
        }

        if (std::isnan(relative_pose.position.y())) {
            relative_pose.position.y() = 0;
        }

        if (std::isnan(relative_pose.position.z())) {
            if (simmode_name == AutonomySimSettings::kSimModeTypeMultirotor) {
                relative_pose.position.z() = 0;
            } else {
                relative_pose.position.z() = -1; // a little bit above for cars
            }
        }

        float pitch, roll, yaw;
        pitch = !std::isnan(rotation.pitch) ? rotation.pitch : 0;
        roll = !std::isnan(rotation.roll) ? rotation.roll : 0;
        yaw = !std::isnan(rotation.yaw) ? rotation.yaw : 0;
        relative_pose.orientation =
            VectorMath::toQuaternion(Utils::degreesToRadians(pitch), // pitch - rotation around Y axis
                                     Utils::degreesToRadians(roll),  // roll  - rotation around X axis
                                     Utils::degreesToRadians(yaw));  // yaw   - rotation around Z axis
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif
