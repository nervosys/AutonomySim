// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_physics_ExternalPhysicsEngine_hpp
#define autonomylib_physics_ExternalPhysicsEngine_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include "common/SteppableClock.hpp"
#include "physics/PhysicsEngineBase.hpp"

#include <cinttypes>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace nervosys {
namespace autonomylib {

class ExternalPhysicsEngine : public PhysicsEngineBase {

  public:
    ExternalPhysicsEngine() {}

    //*** Start: UpdatableState implementation ***//
    virtual void resetImplementation() override {}

    virtual void update() override {
        PhysicsEngineBase::update();

        for (PhysicsBody *body_ptr : *this) {
            body_ptr->updateKinematics();
            body_ptr->update();
        }
    }
    virtual void reportState(StateReporter &reporter) override {
        for (PhysicsBody *body_ptr : *this) {
            reporter.writeValue("ExternalPhysicsEngine", true);
            reporter.writeValue("Is Grounded", body_ptr->isGrounded());
        }
        // call base
        UpdatableObject::reportState(reporter);
    }
    //*** End: UpdatableState implementation ***//
};

} // namespace autonomylib
} // namespace nervosys

#endif
