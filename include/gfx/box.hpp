#pragma once

#include <raylib.h>

#include "base/type.hpp"
#include "gfx/layout.hpp"
#include "gfx/point.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;
struct Box {
    I32 x{0};
    I32 y{0};
    I32 width{0};
    I32 height{0};

    // NOTE: Checks the validity of the box parameters.
    bool check() const;

    // NOTE: Returns the geometrical center of the box.
    Point center() const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding(I32 padding) const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding_x(I32 padding) const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding_y(I32 padding) const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding_left(I32 padding) const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding_right(I32 padding) const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding_top(I32 padding) const;

    // NOTE: Returns a new box with the padding applied.
    Box with_padding_bottom(I32 padding) const;

    // NOTE: Draws the box with the provided color as it's background; returns
    // itself.
    Box with_draw_background(Color color) const;

    // NOTE: Draws the box with the border; returns itself.
    Box with_draw_border(Color color, I32 thinkness = 1) const;

    // NOTE: Draws the box with the border; returns itself.
    Box with_draw_border_left(Color color, I32 thinkness = 1) const;

    // NOTE: Draws the box with the border; returns itself.
    Box with_draw_border_right(Color color, I32 thinkness = 1) const;

    // NOTE: Draws the box with the border; returns itself.
    Box with_draw_border_top(Color color, I32 thinkness = 1) const;

    // NOTE: Draws the box with the border; returns itself.
    Box with_draw_border_bottom(Color color, I32 thinkness = 1) const;

    // NOTE: Draws the horizontal line in the box; returns itself.
    Box with_draw_line_horizontal(Layout layout, Color color,
                                  I32 thinkness = 1) const;

    // NOTE: Draws the horizontal line in the box; returns itself.
    Box with_draw_line_vertical(Layout layout, Color color,
                                I32 thinkness = 1) const;

    // NOTE: Returns Raylib's Rectangle.
    Rectangle rectangle() const;
};
}  // namespace nbody::gfx
