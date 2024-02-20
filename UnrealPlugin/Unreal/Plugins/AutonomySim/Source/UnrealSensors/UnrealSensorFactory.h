// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NedTransform.h"
#include "sensors/SensorFactory.hpp"
#include <memory>

class UnrealSensorFactory : public nervosys::autonomylib::SensorFactory {
  public:
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;

  public:
    UnrealSensorFactory(AActor *actor, const NedTransform *ned_transform);
    virtual ~UnrealSensorFactory() {}
    void setActor(AActor *actor, const NedTransform *ned_transform);
    virtual std::shared_ptr<nervosys::autonomylib::SensorBase>
    createSensorFromSettings(const AutonomySimSettings::SensorSetting *sensor_setting) const override;

  private:
    AActor *actor_;
    const NedTransform *ned_transform_;
};
