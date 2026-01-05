#include "base/type.hpp"
#include "math/collision.hpp"
#include "math/vec.hpp"

namespace nbody::math {
using namespace nbody::base::type;

template <FloatT Float>
bool check_collision2_rect_point(const Vec2T<Float> rect_corner_a, const Vec2T<Float> rect_corner_b,
                                 const Vec2T<Float> point) {
    if (point.x > std::max(rect_corner_a.x, rect_corner_b.x)) {
        return false;
    }

    if (point.x < std::min(rect_corner_a.x, rect_corner_b.x)) {
        return false;
    }

    if (point.y > std::max(rect_corner_a.y, rect_corner_b.y)) {
        return false;
    }

    if (point.y < std::min(rect_corner_a.y, rect_corner_b.y)) {
        return false;
    }

    return true;
}

template bool check_collision2_rect_point(const Vec2F32 rect_corner_a, const Vec2F32 rect_corner_b,
                                          const Vec2F32 point);
template bool check_collision2_rect_point(const Vec2F64 rect_corner_a, const Vec2F64 rect_corner_b,
                                          const Vec2F64 point);

template <FloatT Float>
bool check_collision2_rect_circle(const Vec2T<Float> rect_corner_a,
                                  const Vec2T<Float> rect_corner_b, const Vec2T<Float> center,
                                  Float radius) {
    if (center.x + radius > std::max(rect_corner_a.x, rect_corner_b.x)) {
        return false;
    }

    if (center.x + radius < std::min(rect_corner_a.x, rect_corner_b.x)) {
        return false;
    }

    if (center.y + radius > std::max(rect_corner_a.y, rect_corner_b.y)) {
        return false;
    }

    if (center.y + radius < std::min(rect_corner_a.y, rect_corner_b.y)) {
        return false;
    }

    if (std::sqrt(center.x + radius) > std::max(rect_corner_a.x, rect_corner_b.x)) {
        return false;
    }

    if (std::sqrt(center.x + radius) < std::min(rect_corner_a.x, rect_corner_b.x)) {
        return false;
    }

    if (std::sqrt(center.y + radius) > std::max(rect_corner_a.y, rect_corner_b.y)) {
        return false;
    }

    if (std::sqrt(center.y + radius) < std::min(rect_corner_a.y, rect_corner_b.y)) {
        return false;
    }

    return true;
}

template bool check_collision2_rect_circle(const Vec2F32 rect_corner_a, const Vec2F32 rect_corner_b,
                                           const Vec2F32 point, F32 radius);
template bool check_collision2_rect_circle(const Vec2F64 rect_corner_a, const Vec2F64 rect_corner_b,
                                           const Vec2F64 point, F64 radius);
}  // namespace nbody::math
