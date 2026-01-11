#pragma once

#include <raylib.h>

#include <span>
#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"
#include "sim/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

enum class Layout {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
};

bool layout_is_top(Layout layout);
bool layout_is_bottom(Layout layout);
bool layout_is_left(Layout layout);
bool layout_is_right(Layout layout);

void draw_ui_grid(const Grid& grid);
void draw_ui_text(const Box& box, Layout layout, I32 size, const std::string& text,
                  Color color = WHITE);
void draw_ui_text_fit(const Box& box, Layout layout, const std::string& text, Color color = WHITE);

template <FloatT Float>
void draw_sim_bodies(const Camera<Float> camera, Float scale,
                     std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies);
}  // namespace nbody::gfx
