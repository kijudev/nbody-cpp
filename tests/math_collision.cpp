#include "base/type.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "math/collision.hpp"

using namespace nbody::base::type;
using namespace nbody::math;

template <SignedNumberT Number>
bool rect_point(Number rx, Number ry, Number rwidth, Number rheight, Number px, Number py) {
    return check_collision2_rect_point<Number>(rx, ry, rwidth, rheight, px, py);
}

template <SignedNumberT Number>
bool rect_circle(Number rx, Number ry, Number rwidth, Number rheight, Number cx, Number cy,
                 Number cr) {
    return check_collision2_rect_circle<Number>(rx, ry, rwidth, rheight, cx, cy, cr);
}

TEST_CASE("Collision; rect vs point; U32 - integer cases") {
    const I32 rx = 0;
    const I32 ry = 0;
    const I32 rw = 10;
    const I32 rh = 5;

    SUBCASE("point strictly inside") { CHECK(rect_point<I32>(rx, ry, rw, rh, 5, 2)); }

    SUBCASE("point on edges and corners") {
        CHECK(rect_point<I32>(rx, ry, rw, rh, rx, ry));            // top-left
        CHECK(rect_point<I32>(rx, ry, rw, rh, rx + rw, ry));       // top-right
        CHECK(rect_point<I32>(rx, ry, rw, rh, rx, ry + rh));       // bottom-left
        CHECK(rect_point<I32>(rx, ry, rw, rh, rx + rw, ry + rh));  // bottom-right corner
        CHECK(rect_point<I32>(rx, ry, rw, rh, 3, ry + rh));        // on bottom edge
    }

    SUBCASE("points outside (left/right/top/bottom)") {
        CHECK(!rect_point<I32>(rx, ry, rw, rh, -1, 2));  // left
        CHECK(!rect_point<I32>(rx, ry, rw, rh, 11, 2));  // right
        CHECK(!rect_point<I32>(rx, ry, rw, rh, 5, -1));  // above / top
        CHECK(!rect_point<I32>(rx, ry, rw, rh, 5, 6));   // below / bottom
    }
}

TEST_CASE("Collision; rect vs point; F32 - floating point cases") {
    const F32 rx = -1.0f;
    const F32 ry = -2.0f;
    const F32 rw = 3.5f;
    const F32 rh = 4.0f;

    SUBCASE("inside and boundary") {
        CHECK(rect_point<F32>(rx, ry, rw, rh, -0.5f, 0.0f));       // inside
        CHECK(rect_point<F32>(rx, ry, rw, rh, rx + rw, ry + rh));  // on bottom-right corner
        CHECK(rect_point<F32>(rx, ry, rw, rh, rx, ry));            // top-left corner
    }

    SUBCASE("outside checks") {
        CHECK(!rect_point<F32>(rx, ry, rw, rh, -2.0f, 0.0f));            // left outside
        CHECK(!rect_point<F32>(rx, ry, rw, rh, rx + rw + 0.1f, 0.0f));   // right outside
        CHECK(!rect_point<F32>(rx, ry, rw, rh, 0.0f, ry - 0.1f));        // above / top
        CHECK(!rect_point<F32>(rx, ry, rw, rh, 0.0f, ry + rh + 0.01f));  // below / bottom
    }
}

TEST_CASE("Collision; rect vs circle; F64 - basic cases") {
    const F64 rx = 0.0;
    const F64 ry = 0.0;
    const F64 rw = 10.0;
    const F64 rh = 10.0;

    SUBCASE("circle fully inside or overlapping") {
        CHECK(rect_circle<F64>(rx, ry, rw, rh, 5.0, 5.0, 1.0));    // fully inside
        CHECK(rect_circle<F64>(rx, ry, rw, rh, 9.5, 5.0, 1.0));    // overlapping right edge
        CHECK(rect_circle<F64>(rx, ry, rw, rh, 10.5, 10.5, 1.0));  // overlapping corner
    }

    SUBCASE("circle outside / touching") {
        CHECK(!rect_circle<F64>(rx, ry, rw, rh, 20.0, 20.0, 1.0));  // far away
        CHECK(rect_circle<F64>(rx, ry, rw, rh, 11.0, 5.0, 1.0));    // touching right edge
        CHECK(!rect_circle<F64>(rx, ry, rw, rh, 12.1, 5.0, 1.0));   // outside right edge
    }
}
