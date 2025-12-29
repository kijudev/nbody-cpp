#pragma once

#include "base/type.hpp"

namespace nbody {
// NOTE: Helper to pick a reasonable default epsilon for approximate comparisons.
template <FloatT F>
constexpr F default_epsilon() noexcept {
    if constexpr (std::is_same_v<F, F32>) {
        return static_cast<F>(1e-6f);
    } else {
        return static_cast<F>(1e-12);
    }
}
}  // namespace nbody
