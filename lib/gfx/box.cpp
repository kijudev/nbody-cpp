#include "base/assert.hpp"
#include "base/type.hpp"
#include "gfx/box.hpp"
#include "gfx/point.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

bool Box::check() const { return top_left.x <= bottom_right.x && top_left.y <= bottom_right.y; }

I32 Box::width() const {
    ASSERT(check(), "Bad dimentions");
    return bottom_right.x - top_left.x;
}
I32 Box::height() const {
    ASSERT(check(), "Bad dimentions");
    return bottom_right.y - top_left.y;
}

Point Box::center() const {
    ASSERT(check(), "Bad dimentions");
    return {
        .x = (top_left.x + bottom_right.x) / 2,
        .y = (top_left.y + bottom_right.y) / 2,

    };
}

I32 Box::x() const {
    ASSERT(check(), "Bad dimentions");
    return top_left.x;
}

I32 Box::y() const {
    ASSERT(check(), "Bad dimentions");
    return top_left.y;
}
}  // namespace nbody::gfx
