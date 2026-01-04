#pragma once

#include "base/type.hpp"

namespace nbody::math::impl {
using namespace nbody::base::type;

// NOTE: Helper to pick a reasonable default epsilon for approximate comparisons.
template <FloatT Float>
constexpr Float default_epsilon() noexcept {
    if constexpr (std::is_same_v<Float, F32>) {
        return static_cast<Float>(1e-6f);
    } else {
        return static_cast<Float>(1e-12);
    }
}
}  // namespace nbody::math::impl
