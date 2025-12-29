#pragma once

#include "base/type.hpp"

namespace nbody {
// NOTE: Helper to pick a reasonable default epsilon for approximate comparisons.
template <FloatingPointT T>
constexpr T default_epsilon() noexcept {
    if constexpr (std::is_same_v<T, F32>) {
        return static_cast<T>(1e-6f);
    } else {
        return static_cast<T>(1e-12);
    }
}
}  // namespace nbody
