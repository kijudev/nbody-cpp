#pragma once

#include <raygui.h>
#include <raylib.h>

#include <optional>
#include <span>
#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "sim/type.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

template <FloatT Float>
std::string format_time(Float factor, Float year, Float day, Float hour,
                        Float minute);

template <FloatT Float>
std::optional<USize> get_body_at_mouse_position_au(
    gfx::Camera<Float>                                      camera,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies);

}  // namespace nbody::scenario::impl
