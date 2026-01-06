#pragma once

#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

template <FloatT Float>
void draw_ruler(const Camera<Float>& camera, const std::string& text);
}  // namespace nbody::gfx
