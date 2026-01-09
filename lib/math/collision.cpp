#include <cmath>

#include "math/collision.hpp"

namespace nbody::math {
using namespace nbody::base::type;

template <SignedNumberT Number>
bool check_collision2_rect_point(Number rx, Number ry, Number rwidth, Number rheight, Number px,
                                 Number py) {
    if (px < rx) {
        return false;
    }

    if (px > rx + rwidth) {
        return false;
    }

    if (py < ry) {
        return false;
    }

    if (py > py + rheight) {
        return false;
    }

    return true;
}

template bool check_collision2_rect_point(I8 rx, I8 ry, I8 rwidth, I8 rheight, I8 px, I8 py);
template bool check_collision2_rect_point(I16 rx, I16 ry, I16 rwidth, I16 rheight, I16 px, I16 py);
template bool check_collision2_rect_point(I32 rx, I32 ry, I32 rwidth, I32 rheight, I32 px, I32 py);
template bool check_collision2_rect_point(I64 rx, I64 ry, I64 rwidth, I64 rheight, I64 px, I64 py);
template bool check_collision2_rect_point(F32 rx, F32 ry, F32 rwidth, F32 rheight, F32 px, F32 py);
template bool check_collision2_rect_point(F64 rx, F64 ry, F64 rwidth, F64 rheight, F64 px, F64 py);

template <SignedNumberT Number>
bool check_collision2_rect_circle(Number rx, Number ry, Number rwidth, Number rheight, Number cx,
                                  Number cy, Number cr) {
    // NOTE: Temporary variables to set the edges for testing.
    Number tx = cx;
    Number ty = cy;

    // NOTE: Find out which edge is the closest for the x axis.
    if (cx < rx) {
        tx = rx;
    } else if (cx > rx + rwidth) {
        tx = rx + rwidth;
    }

    // NOTE: Find out which edge is the closest for the y axis.
    if (cy < ry) {
        ty = ry;
    } else if (cy > ry + rheight) {
        ty = ry + rheight;
    }

    // NOTE: Calculate the distance from the closest edges.
    Number distx = cx - tx;
    Number disty = cy - ty;
    Number dist  = std::sqrt((distx * distx) + (disty * disty));

    if (dist <= cr) {
        return true;
    }

    return false;
}

template bool check_collision2_rect_circle(I8 rx, I8 ry, I8 width, I8 rheight, I8 cx, I8 cy, I8 cr);
template bool check_collision2_rect_circle(I16 rx, I16 ry, I16 width, I16 rheight, I16 cx, I16 cy,
                                           I16 cr);
template bool check_collision2_rect_circle(I32 rx, I32 ry, I32 width, I32 rheight, I32 cx, I32 cy,
                                           I32 cr);
template bool check_collision2_rect_circle(I64 rx, I64 ry, I64 width, I64 rheight, I64 cx, I64 cy,
                                           I64 cr);
template bool check_collision2_rect_circle(F32 rx, F32 ry, F32 width, F32 rheight, F32 cx, F32 cy,
                                           F32 cr);
template bool check_collision2_rect_circle(F64 rx, F64 ry, F64 width, F64 rheight, F64 cx, F64 cy,
                                           F64 cr);
}  // namespace nbody::math
