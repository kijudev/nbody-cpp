#pragma once

#include "base/type.hpp"
#include "type.hpp"

namespace nbody {

template <FloatT Float>
void integrate2_euler(Body2T<Float> body, Float dt) {
    body.pos = body.pos.add(body.vel.scale(dt));
    body.vel = body.vel.add(body.acc.scale(dt));
}

}  // namespace nbody
