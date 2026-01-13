#include "base/assert.hpp"
#include "base/type.hpp"
#include "gfx/grid.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

Grid Grid::from_box(const Box& box, I32 cols, I32 rows) {
    return Grid{
        .x      = box.x,
        .y      = box.y,
        .width  = box.width,
        .height = box.height,
        .cols   = cols,
        .rows   = rows,
    };
}

Box Grid::span(I32 start_col, I32 end_col, I32 start_row, I32 end_row) const {
    return Box{.x      = start_col * col_size() + x,
               .y      = start_row * row_size() + y,
               .width  = (end_col - start_col + 1) * col_size(),
               .height = (end_row - start_row + 1) * row_size()};
}

I32 Grid::col_size() const {
    ASSERT(cols > 0, "The number of columns has to be greater than 0");
    return width / cols;
}

I32 Grid::row_size() const {
    ASSERT(rows > 0, "The number of rows has to be greater than 0");
    return height / rows;
}

std::vector<Box> Grid::row_boxes() const {
    std::vector<Box> boxes;
    boxes.reserve(rows);

    I32 row_height = height / rows;
    for (I32 i = 0; i < cols; ++i) {
        boxes.emplace_back(Box{
            .x      = x,
            .y      = y + (row_height * i),
            .width  = width,
            .height = row_height,
        });
    }

    return boxes;
}

std::vector<Box> Grid::col_boxes() const {
    std::vector<Box> boxes;
    boxes.reserve(cols);

    I32 col_width = height / rows;
    for (I32 i = 0; i < cols; ++i) {
        boxes.emplace_back(Box{
            .x      = x + (col_width * i),
            .y      = y,
            .width  = col_width,
            .height = height,
        });
    }

    return boxes;
}
}  // namespace nbody::gfx
