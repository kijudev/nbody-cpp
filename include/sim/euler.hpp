#pragma once

#include "base/type.hpp"
#include "type.hpp"

namespace nbody {

template <FloatT Float>
void euler2(Body2T<Float>& body, Float dt) {
    body.pos = body.pos.add(body.vel.scale(dt));
    body.vel = body.vel.add(body.acc.scale(dt));
}

}  // namespace nbody
