#pragma once

#include "base/type.hpp"
#include "type.hpp"

namespace nbody2 {

template <FloatT Float>
void euler_integrate_body(BodyT<Float>& body, Float dt) {
    body.pm.pos = body.pm.pos.add(body.vel.scale(dt));
    body.vel    = body.vel.add(body.acc.scale(dt));
}

}  // namespace nbody2
