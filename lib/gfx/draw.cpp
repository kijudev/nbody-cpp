
#include <raylib.h>

#include <cstdlib>
#include <string>

#include "base/type.hpp"
#include "gfx/draw.hpp"
#include "gfx/grid.hpp"
#include "math/vec.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

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

template <FloatT Float>
void draw_ui_ruler(const Camera<Float>& camera, const std::string& text) {
    I32               width   = 256;
    nbody::gfx::Point point_a = {(camera.screen_width / 2) - (width / 2),
                                 camera.screen_height - 32};
    nbody::gfx::Point point_b = {(camera.screen_width / 2) + (width / 2),
                                 camera.screen_height - 32};

    DrawRectangle(point_a.x, point_a.y, width, 2, WHITE);
    DrawRectangle(point_a.x, point_a.y - 16, 2, 32, WHITE);
    DrawRectangle(point_b.x, point_b.y - 16, 2, 32, WHITE);

    math::Vec2T<Float> world_a      = camera.screen_to_world(point_a);
    math::Vec2T<Float> world_b      = camera.screen_to_world(point_b);
    Float              distance     = world_a.distance(world_b);
    std::string        display_text = std::to_string(distance) + text;

    DrawText(display_text.c_str(), point_a.x + 48, point_a.y - 32, 24, WHITE);
}

template void draw_ui_ruler(const Camera<F32>& camera, const std::string& text);
template void draw_ui_ruler(const Camera<F64>& camera, const std::string& text);

void draw_ui_grid(const Grid& grid) {
    for (I32 i = 0; i < grid.cols; ++i) {
        DrawLine(i * grid.col_size(), 0, i * grid.col_size(), grid.height, WHITE);
    }

    for (I32 i = 0; i < grid.rows; ++i) {
        DrawLine(0, i * grid.row_size(), grid.width, i * grid.row_size(), WHITE);
    }
}

void draw_ui_text(const Box& box, Layout layout, I32 size, const std::string& text) {
    I32 text_width = MeasureText(text.c_str(), size);
    I32 padding_x  = 0;
    I32 padding_y  = 0;

    if (layout_is_left(layout)) {
        padding_x = 0;
    } else if (layout_is_right(layout)) {
        padding_x = (box.width - text_width);
    } else {
        padding_x = (box.width - text_width) / 2;
    }

    if (layout_is_top(layout)) {
        padding_y = 0;
    } else if (layout_is_bottom(layout)) {
        padding_y = (box.height - size);
    } else {
        padding_y = (box.height - size) / 2;
    }

    DrawText(text.c_str(), box.x + padding_x, box.y + padding_y, size, WHITE);
}
}  // namespace nbody::gfx
