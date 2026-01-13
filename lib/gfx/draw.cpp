
#include <raylib.h>

#include <cstdlib>
#include <string>

#include "base/type.hpp"
#include "gfx/draw.hpp"
#include "gfx/grid.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

void draw_grid(const Grid& grid) {
    for (I32 i = 0; i < grid.cols; ++i) {
        DrawLine(i * grid.col_size(), 0, i * grid.col_size(), grid.height,
                 WHITE);
    }

    for (I32 i = 0; i < grid.rows; ++i) {
        DrawLine(0, i * grid.row_size(), grid.width, i * grid.row_size(),
                 WHITE);
    }
}

void draw_text(const Box& box, Layout layout, I32 size, const std::string& text,
               Color color) {
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

    DrawText(text.c_str(), box.x + padding_x, box.y + padding_y, size, color);
}

void draw_text_fit(const Box& box, Layout layout, const std::string& text,
                   Color color) {
    I32 size       = 1;
    I32 text_width = MeasureText(text.c_str(), size);

    while (text_width < box.width && size < box.height) {
        size += 1;
        text_width = MeasureText(text.c_str(), size);
    }

    size = std::max(1, size - 1);

    draw_text(box, layout, size, text, color);
}
}  // namespace nbody::gfx
