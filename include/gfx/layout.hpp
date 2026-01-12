#pragma once

namespace nbody::gfx {
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
}  // namespace nbody::gfx
