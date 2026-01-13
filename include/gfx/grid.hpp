#pragma once

#include <vector>

#include "base/type.hpp"
#include "gfx/box.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

struct Grid {
    I32 x{0};
    I32 y{0};
    I32 width{0};
    I32 height{0};
    I32 cols{1};
    I32 rows{1};

    static Grid from_box(const Box& box, I32 cols = 1, I32 rows = 1);

    Box span(I32 start_col, I32 end_col, I32 start_row, I32 end_row) const;

    I32 col_size() const;
    I32 row_size() const;

    std::vector<Box> row_boxes() const;
    std::vector<Box> col_boxes() const;
};
}  // namespace nbody::gfx
