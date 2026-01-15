// ==============================================================================
// box.hpp
// UI primitive for rectangular regions. Provides geometry, padding, and drawing
// utilities for UI layout and rendering. Used across the whole project.
// ==============================================================================

#pragma once

#include <raylib.h>

#include "base/type.hpp"
#include "gfx/layout.hpp"
#include "gfx/point.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// ==============================================================================
// Box
// ==============================================================================
// Represents a rectangular region in screen/UI space. Provides helpers for
// geometry, padding, and drawing. All 'with_*' methods return a new Box with
// the modification applied, enabling method chaining for UI layout code.
// ==============================================================================

struct Box {
    I32 x{0};
    I32 y{0};
    I32 width{0};
    I32 height{0};

    // Checks the validity of the box parameters (width/height > 0).
    bool check() const;

    // Returns the geometrical center of the box as a Point.
    Point center() const;

    // Padding helpers: return a new box with the specified padding applied.
    Box with_padding(I32 padding) const;
    Box with_padding_x(I32 padding) const;
    Box with_padding_y(I32 padding) const;
    Box with_padding_left(I32 padding) const;
    Box with_padding_right(I32 padding) const;
    Box with_padding_top(I32 padding) const;
    Box with_padding_bottom(I32 padding) const;

    // Drawing helpers: return a new box after drawing background or borders.
    Box with_draw_background(Color color) const;
    Box with_draw_border(Color color, I32 thinkness = 1) const;
    Box with_draw_border_left(Color color, I32 thinkness = 1) const;
    Box with_draw_border_right(Color color, I32 thinkness = 1) const;
    Box with_draw_border_top(Color color, I32 thinkness = 1) const;
    Box with_draw_border_bottom(Color color, I32 thinkness = 1) const;

    // Draws a horizontal or vertical line in the box at the specified layout.
    Box with_draw_line_horizontal(Layout layout, Color color,
                                  I32 thinkness = 1) const;
    Box with_draw_line_vertical(Layout layout, Color color,
                                I32 thinkness = 1) const;

    // Returns Raylib's Rectangle representation of this box.
    Rectangle rectangle() const;
};

}  // namespace nbody::gfx
