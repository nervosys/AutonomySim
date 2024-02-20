// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "GameFramework/Actor.h"
#include "NedTransform.h"
#include "common/Common.hpp"
#include "sensors/distance/DistanceSimple.hpp"

class UnrealDistanceSensor : public nervosys::autonomylib::DistanceSimple {
  public:
    typedef nervosys::autonomylib::AutonomySimSettings AutonomySimSettings;

  public:
    UnrealDistanceSensor(const AutonomySimSettings::DistanceSetting &setting, AActor *actor,
                         const NedTransform *ned_transform);

  protected:
    virtual nervosys::autonomylib::real_T getRayLength(const nervosys::autonomylib::Pose &pose) override;

  private:
    using Vector3r = nervosys::autonomylib::Vector3r;
    using VectorMath = nervosys::autonomylib::VectorMath;

  private:
    AActor *actor_;
    const NedTransform *ned_transform_;
};