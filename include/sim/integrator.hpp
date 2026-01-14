#pragma once

#include "base/type.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
void integrate_body_euler(BodyT<Float>& body, Float dt);

template <FloatT Float>
void integrate_body_verlet(BodyT<Float>& body, Float dt);

// NOTE: Proper Velocity Verlet that uses both old and new acceleration
// Call this AFTER computing new acceleration in the same timestep
template <FloatT Float>
void integrate_body_verlet_proper(BodyT<Float>& body, Float dt, 
                                   const math::Vec2T<Float>& old_acc);
}  // namespace nbody::sim
