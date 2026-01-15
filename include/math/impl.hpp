// ==============================================================================
// impl.hpp
// Math module internal utilities for nbody project.
// Provides helpers for floating-point comparisons and other math internals.
// ==============================================================================

#pragma once

#include "base/type.hpp"

// ==============================================================================
// Implementation Utilities
// ==============================================================================

namespace nbody::math::impl {
using namespace nbody::base::type;

// NOTE: Returns a reasonable default epsilon for approximate floating-point
// comparisons. F32 uses 1e-6, F64 uses 1e-12.
template <FloatT Float>
constexpr Float default_epsilon() noexcept {
    if constexpr (std::is_same_v<Float, F32>) {
        return static_cast<Float>(1e-6f);
    } else {
        return static_cast<Float>(1e-12);
    }
}
}  // namespace nbody::math::impl
