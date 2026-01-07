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

    return Box{
        .x = col_a * col_size(),
        .y = row_a * row_size(),
        .width = (col_b - col_a + 1) * col_size(),
        .height = (row_b - row_a + 1) * row_size()
    };
}

Box Grid::col(I32 col, I32 rows) const {
    ASSERT(col >= 0, "Column index has to be greater or equal to 0");
    ASSERT(rows > 0, "The number of rows has to be greater than 0");

    return Box{
        .x = col * col_size(),
        .y = 0,
        .width = col_size(),
        .height = rows * row_size()
    };
}

Box Grid::row(I32 row, I32 cols) const {
    ASSERT(row >= 0, "Row index has to be greater or equal to 0");
    ASSERT(cols > 0, "The number of columns has to be greater than 0");

    return Box{
        .x = 0,
        .y = row * row_size(),
        .width = cols * col_size(),
        .height = row_size()
    };
}

std::vector<Box> Grid::all_cols() const {
    std::vector<Box> cols_vec{};
    cols_vec.reserve(cols);

    for (I32 i = 0; i < cols; ++i) {
        cols_vec.push_back(col(i, rows));
    }

    return cols_vec;
}

std::vector<Box> Grid::all_rows() const {
    std::vector<Box> rows_vec{};
    rows_vec.reserve(rows);

    for (I32 i = 0; i < rows; ++i) {
        rows_vec.push_back(row(i, cols));
    }

    return rows_vec;
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
