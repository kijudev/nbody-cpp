// ==============================================================================
// grid.hpp
// Grid layout system for UI and drawing. Provides a way to subdivide a box into
// a grid of rows and columns, and to extract sub-boxes or spans for layout.
// ==============================================================================

#pragma once

#include <vector>

#include "base/type.hpp"
#include "gfx/box.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// ==============================================================================
// Grid Layout Type
// ==============================================================================

// Represents a rectangular grid subdivided into rows and columns.
struct Grid {
    I32 x{0};       // Top-left x coordinate
    I32 y{0};       // Top-left y coordinate
    I32 width{0};   // Total width of the grid
    I32 height{0};  // Total height of the grid
    I32 cols{1};    // Number of columns
    I32 rows{1};    // Number of rows

    // Create a grid from a box, specifying number of columns and rows.
    static Grid from_box(const Box& box, I32 cols = 1, I32 rows = 1);

    // Returns a box spanning the specified columns and rows.
    Box span(I32 start_col, I32 end_col, I32 start_row, I32 end_row) const;

    // Returns the width of a single column.
    I32 col_size() const;

    // Returns the height of a single row.
    I32 row_size() const;

    // Returns a vector of boxes, one for each row.
    std::vector<Box> row_boxes() const;

    // Returns a vector of boxes, one for each column.
    std::vector<Box> col_boxes() const;
};
}  // namespace nbody::gfx
