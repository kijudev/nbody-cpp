#pragma once

#include <concepts>

#include "base/type.hpp"
#include "body2.hpp"

namespace nbody {
template <typename Float, typename Fn>
concept Integrate2FnT = FloatingPointT<Float> && requires(Fn&& fn, Body2T<Float>& body, Float dt) {
    { fn(body, dt) } -> std::same_as<void>;
};

template <typename Float>
    requires FloatingPointT<Float>
void integrate2_euler(Body2T<Float>& body, Float dt) {
    body.vel.mut_add(body.acc.scale(dt));
    body.pos.mut_add(body.vel.scale(dt));
}

}  // namespace nbody
