#include "base/assert.hpp"
#include "base/type.hpp"
#include "gfx/grid.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

Box Grid::span(I32 col_a, I32 row_a, I32 col_b, I32 row_b) const {
    ASSERT(col_a >= 0, "Column index has to be greater or equal to 0");
    ASSERT(col_b >= 0, "Column index has to be greater or equal to 0");
    ASSERT(row_a >= 0, "Row index has to be greater or equal to 0");
    ASSERT(row_b >= 0, "Row index has to be greater or equal to 0");
    ASSERT(col_a <= col_b, "Invalid params");
    ASSERT(row_a <= row_b, "Invalid params");

    const Point top_left = {
        .x = col_a * col_size(),
        .y = row_a * row_size(),
    };
    const Point bottom_right = {
        .x = col_b * col_size(),
        .y = row_b * row_size(),
    };

    return Box{
        .top_left     = top_left,
        .bottom_right = bottom_right,
    };
}

I32 Grid::col_size() const {
    ASSERT(cols > 0, "The number of columns has to be greater than 0");
    return width / cols;
}

I32 Grid::row_size() const {
    ASSERT(rows > 0, "The number of rows has to be greater than 0");
    return height / rows;
}
}  // namespace nbody::gfx
