#include "gfx/layout.hpp"

namespace nbody::gfx {

bool layout_is_top(Layout layout) {
    return layout == Layout::TopLeft || layout == Layout::TopCenter || layout == Layout::TopRight;
}

bool layout_is_bottom(Layout layout) {
    return layout == Layout::BottomLeft || layout == Layout::BottomCenter ||
           layout == Layout::BottomRight;
}

bool layout_is_right(Layout layout) {
    return layout == Layout::TopRight || layout == Layout::CenterRight ||
           layout == Layout::BottomRight;
}
bool layout_is_left(Layout layout) {
    return layout == Layout::TopLeft || layout == Layout::CenterLeft ||
           layout == Layout::BottomLeft;
}
}  // namespace nbody::gfx
