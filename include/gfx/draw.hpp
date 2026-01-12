#pragma once

#include <raylib.h>

#include <span>
#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"
#include "sim/type.hpp"
#include "gfx/layout.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;


void draw_ui_grid(const Grid& grid);
void draw_ui_text(const Box& box, Layout layout, I32 size, const std::string& text,
                  Color color = WHITE);
void draw_ui_text_fit(const Box& box, Layout layout, const std::string& text, Color color = WHITE);

template <FloatT Float>
void draw_sim_bodies(const Camera<Float> camera, Float scale,
                     std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies);
}  // namespace nbody::gfx
