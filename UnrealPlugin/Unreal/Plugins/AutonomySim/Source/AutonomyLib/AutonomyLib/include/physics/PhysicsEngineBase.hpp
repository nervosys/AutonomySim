// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_physics_PhysicsEngineBase_hpp
#define autonomylib_physics_PhysicsEngineBase_hpp

#include "PhysicsBody.hpp"
#include "common/Common.hpp"
#include "common/UpdatableContainer.hpp"

namespace nervosys {
namespace autonomylib {

class PhysicsEngineBase : public UpdatableContainer<PhysicsBody *> {

  public:
    virtual void update() override { UpdatableObject::update(); }

    virtual void reportState(StateReporter &reporter) override {
        unused(reporter);
        // default nothing to report for physics engine
    }

    virtual void setWind(const Vector3r &wind) { unused(wind); };
    virtual void setExtForce(const Vector3r &ext_force) { unused(ext_force); };
};

} // namespace autonomylib
} // namespace nervosys

#endif
