#pragma once

#include <concepts>

#include "base/type.hpp"
#include "body2.hpp"

namespace nbody {
template <typename F, typename Fn>
concept Integrate2FnT = FloatT<F> && requires(Fn&& fn, Body2T<F>& body, F dt) {
    { fn(body, dt) } -> std::same_as<void>;
};

template <typename Float>
    requires FloatT<Float>
void integrate2_euler(Body2T<Float>& body, Float dt) {
    body.vel = body.vel.add(body.acc.scale(dt));
    body.pos = body.pos.add(body.vel.scale(dt));
}

}  // namespace nbody
