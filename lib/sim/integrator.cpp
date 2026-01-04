#include "sim/integrator.hpp"

#pragma once

#include "base/type.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
void integrate_body_euler(BodyT<Float>& body, Float dt) {
    body.pm.pos = body.pm.pos.add(body.vel.scale(dt));
    body.vel    = body.vel.add(body.acc.scale(dt));
}

template void integrate_body_euler(BodyF32& body, F32 dt);
template void integrate_body_euler(BodyF64& body, F64 dt);

template <FloatT Float>
void integrate_body_verlet(BodyT<Float>& body, Float dt) {
    const Float half  = 0.5;
    const Float dt_sq = dt * dt;

    body.pm.pos = body.pm.pos.add(body.vel.scale(dt)).add(body.acc.scale(half * dt_sq));
    body.vel    = body.vel.add(body.acc.scale(dt));
}

template void integrate_body_verlet(BodyF32& body, F32 dt);
template void integrate_body_verlet(BodyF64& body, F64 dt);
}  // namespace nbody::sim
