// ==============================================================================
// point.hpp
// 2D integer point primitive for UI and graphics positioning.
// Used throughout the gfx module for representing pixel coordinates.
// ==============================================================================

#pragma once

#include "base/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// ==============================================================================
// Point Type
// ==============================================================================
// Represents a 2D point with integer coordinates (x, y).
struct Point {
    I32 x{0}, y{0};
};
 
}  // namespace nbody::gfx
