#pragma once

#include "base/type.hpp"
#include "gfx/point.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;
struct Box {
    Point top_left{0, 0};
    Point bottom_right{0, 0};

    bool  check() const;
    I32   width() const;
    I32   height() const;
    Point center() const;

    // NOTE: For use in raylib's functions.
    I32 x() const;

    // NOTE: For use in raylib's functions.
    I32 y() const;
};
}  // namespace nbody::gfx
