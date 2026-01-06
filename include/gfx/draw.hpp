#pragma once

#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;
void draw_ruler(Point box_corner_a, Point box_corner_b, I32 padding, const std::string& text);
}  // namespace nbody::gfx
