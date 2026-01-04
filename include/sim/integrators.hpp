
#pragma once

#include "base/type.hpp"
#include "type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
void euler_integrate_body(BodyT<Float>& body, Float dt) {
    body.pm.pos = body.pm.pos.add(body.vel.scale(dt));
    body.vel    = body.vel.add(body.acc.scale(dt));
}

template <FloatT Float>
void verlet_integrate_body(BodyT<Float>& body, Float dt) {
    const Float half  = 0.5;
    const Float dt_sq = dt * dt;

    body.pm.pos = body.pm.pos.add(body.vel.scale(dt)).add(body.acc.scale(half * dt_sq));
    body.vel    = body.vel.add(body.acc.scale(dt));
}
}  // namespace nbody::sim
