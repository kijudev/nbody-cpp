// ==============================================================================
// integrator.hpp
// Numerical integration schemes for advancing bodies in n-body simulations.
// Includes Euler, semi-symplectic Euler, Verlet, and proper Velocity Verlet
// methods. Each function is documented with its update formulas.
// ==============================================================================

#pragma once

#include "base/type.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// Euler integration:
// pos = pos + vel * dt
// vel = vel + acc * dt
template <FloatT Float>
void integrate_body_euler(BodyT<Float>& body, Float dt);

// Semi-symplectic Euler integration:
// vel = vel + acc * dt
// pos = pos + vel * dt
template <FloatT Float>
void integrate_body_euler_semi_symplectic(BodyT<Float>& body, Float dt);

// Verlet integration (basic):
// pos = pos + vel * dt + 0.5 * acc * dt^2
// vel = vel + acc * dt
template <FloatT Float>
void integrate_body_verlet(BodyT<Float>& body, Float dt);

// Proper Velocity Verlet integration:
// pos = pos + vel * dt + 0.5 * old_acc * dt^2
// vel = vel + 0.5 * (old_acc + acc) * dt
// Call this AFTER computing new acceleration in the same timestep.
template <FloatT Float>
void integrate_body_verlet_proper(BodyT<Float>& body, Float dt,
                                  const math::Vec2T<Float>& old_acc);
}  // namespace nbody::sim
