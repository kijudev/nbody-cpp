#pragma once

#include <raylib.h>

#include "base/type.hpp"
#include "gfx/point.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;
struct Box {
    I32 x{0};
    I32 y{0};
    I32 width{0};
    I32 height{0};

    bool  check() const;
    Point center() const;

    Box with_padding(I32 padding) const;
    Box with_padding_x(I32 padding) const;
    Box with_padding_y(I32 padding) const;
    Box with_padding_left(I32 padding) const;
    Box with_padding_right(I32 padding) const;
    Box with_padding_top(I32 padding) const;
    Box with_padding_bottom(I32 padding) const;

    Box with_background(Color color, I32 radius) const;
    Box with_border(Color color, I32 radius) const;
    Box with_border_x(Color color, I32 radius) const;
    Box with_border_y(Color color, I32 radius) const;
    Box with_border_left(Color color, I32 radius) const;
    Box with_border_right(Color color, I32 radius) const;
    Box with_border_top(Color color, I32 radius) const;
    Box with_border_bottom(Color color, I32 radius) const;


    Rectangle rectangle() const;
};
}  // namespace nbody::gfx
