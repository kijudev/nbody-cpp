#pragma once

#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"

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

template <FloatT Float>
void draw_ui_ruler(const Camera<Float>& camera, const std::string& text);

void draw_ui_grid(const Grid& grid);
void draw_ui_text(const Box& box, Layout layout, I32 size, const std::string& text);
}  // namespace nbody::gfx
