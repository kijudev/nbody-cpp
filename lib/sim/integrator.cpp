#include "sim/integrator.hpp"

#include "base/type.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
void integrate_body_euler(BodyT<Float>& body, Float dt) {
    // NOTE: Standard Euler (Forward Euler)
    // Update position using old velocity, then update velocity
    // This is NOT symplectic and does not conserve energy well
    body.pos = body.pos.add(body.vel.scale(dt));
    body.vel = body.vel.add(body.acc.scale(dt));
}

template void integrate_body_euler(BodyF32& body, F32 dt);
template void integrate_body_euler(BodyF64& body, F64 dt);

template <FloatT Float>
void integrate_body_verlet(BodyT<Float>& body, Float dt) {
    // NOTE: Velocity Verlet (simplified version)
    // Full Velocity Verlet would need acceleration at t+dt, but our architecture
    // computes all accelerations before integration. This is a reasonable approximation.
    const Float half  = 0.5;
    const Float dt_sq = dt * dt;

    // Update position using current velocity and acceleration
    body.pos = body.pos.add(body.vel.scale(dt)).add(body.acc.scale(half * dt_sq));
    // Update velocity using current acceleration
    // (Ideally would use average of old and new acceleration)
    body.vel = body.vel.add(body.acc.scale(dt));
}

template void integrate_body_verlet(BodyF32& body, F32 dt);
template void integrate_body_verlet(BodyF64& body, F64 dt);

template <FloatT Float>
void integrate_body_verlet_proper(BodyT<Float>& body, Float dt,
                                   const math::Vec2T<Float>& old_acc) {
    // NOTE: Proper Velocity Verlet algorithm
    // This requires the acceleration from the PREVIOUS timestep
    //
    // Algorithm:
    // 1. x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
    // 2. [Compute a(t+dt) - done externally before calling this]
    // 3. v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt

    const Float half  = 0.5;
    const Float dt_sq = dt * dt;

    // Update position using old acceleration
    body.pos = body.pos.add(body.vel.scale(dt)).add(old_acc.scale(half * dt_sq));

    // Update velocity using average of old and new acceleration
    body.vel = body.vel.add(old_acc.add(body.acc).scale(half * dt));
}

template void integrate_body_verlet_proper(BodyF32& body, F32 dt, const math::Vec2F32& old_acc);
template void integrate_body_verlet_proper(BodyF64& body, F64 dt, const math::Vec2F64& old_acc);

}  // namespace nbody::sim
