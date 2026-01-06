
#include <raylib.h>

#include <cstdlib>
#include <string>

#include "base/type.hpp"
#include "gfx/draw.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

void draw_ruler(Point box_corner_a, Point box_corner_b, I32 padding, const std::string& text) {
    Point center = {
        .x = (box_corner_a.x + box_corner_b.x) / 2,
        .y = (box_corner_a.y + box_corner_b.y) / 2,
    };

    I32 width  = std::abs(box_corner_a.x - box_corner_b.x);
    I32 height = std::abs(box_corner_a.y - box_corner_b.y);

    DrawRectangleLines(center.x, center.y, width, height, WHITE);

    Point ruler_center = {
        .x = center.x,
        .y = center.y - (height / 2) + padding,
    };

    I32 ruler_width  = width - (2 * padding);
    I32 ruler_height = 2;

    DrawRectangle(ruler_center.x, ruler_center.y, ruler_width, ruler_height, WHITE);

    (void)text;
}

}  // namespace nbody::gfx
