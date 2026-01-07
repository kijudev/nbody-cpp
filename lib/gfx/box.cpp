#include <raylib.h>

#include "base/assert.hpp"
#include "base/type.hpp"
#include "gfx/box.hpp"
#include "gfx/point.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

bool Box::check() const { return x >= 0 && y >= 0 && width >= 0 && height >= 0; }

Point Box::center() const {
    ASSERT(check(), "Bad dimentions");
    return {
        .x = (x + width) / 2,
        .y = (y + height) / 2,

    };
}

Box Box::with_padding(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= std::min(width, height), "Bad padding");

    return {
        .x      = x + padding,
        .y      = y + padding,
        .width  = width - (2 * padding),
        .height = height - (2 * padding),
    };
}

Box Box::with_padding_x(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= width, "Bad padding");

    return {
        .x      = x + padding,
        .y      = y,
        .width  = width - (2 * padding),
        .height = height,
    };
}

Box Box::with_padding_y(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= height, "Bad padding");

    return {
        .x      = x,
        .y      = y + padding,
        .width  = width,
        .height = height - (2 * padding),
    };
}

Box Box::with_padding_left(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= width, "Bad padding");

    return {
        .x      = x + padding,
        .y      = y,
        .width  = width - padding,
        .height = height,
    };
}

Box Box::with_padding_right(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= width, "Bad padding");

    return {
        .x      = x,
        .y      = y,
        .width  = width - padding,
        .height = height,
    };
}

Box Box::with_padding_top(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= height, "Bad padding");

    return {
        .x      = x,
        .y      = y + padding,
        .width  = width,
        .height = height - padding,
    };
}

Box Box::with_padding_bottom(I32 padding) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(padding >= 0, "Bad padding");
    ASSERT(padding <= height, "Bad padding");

    return {
        .x      = x,
        .y      = y,
        .width  = width,
        .height = height - padding,
    };
}

Box Box::with_background(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleRounded(rectangle(), static_cast<float>(radius), 32, color);

    return *this;
}

Box Box::with_border(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleRoundedLines(rectangle(), static_cast<float>(radius) / 8.0, 32, color);

    return *this;
}

Box Box::with_border_x(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleLinesEx(rectangle(), static_cast<float>(radius) / 8.0, color);

    return *this;
}

Box Box::with_border_y(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleLinesEx(rectangle(), static_cast<float>(radius) / 8.0, color);

    return *this;
}

Box Box::with_border_left(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleLinesEx(rectangle(), static_cast<float>(radius) / 8.0, color);

    return *this;
}

Box Box::with_border_right(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleLinesEx(rectangle(), static_cast<float>(radius) / 8.0, color);

    return *this;
}

Box Box::with_border_top(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleLinesEx(rectangle(), static_cast<float>(radius) / 8.0, color);

    return *this;
}

Box Box::with_border_bottom(Color color = WHITE, I32 radius = 0) const {
    ASSERT(check(), "Bad dimentions");
    ASSERT(radius >= 0, "Bad radius");

    DrawRectangleLinesEx(rectangle(), static_cast<float>(radius) / 8.0, color);

    return *this;
}

Rectangle Box::rectangle() const {
    ASSERT(check(), "Bad dimentions");

    return {
        .x      = static_cast<F32>(x),
        .y      = static_cast<F32>(y),
        .width  = static_cast<F32>(width),
        .height = static_cast<F32>(height),
    };
}

}  // namespace nbody::gfx
