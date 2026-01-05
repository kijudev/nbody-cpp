#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody::math {
using namespace nbody::base::type;

template <FloatT Float>
bool check_collision2_rect_point(const Vec2T<Float> rect_corner_a, const Vec2T<Float> rect_corner_b,
                                 const Vec2T<Float> point);

template <FloatT Float>
bool check_collision2_rect_circle(const Vec2T<Float> rect_corner_a,
                                  const Vec2T<Float> rect_corner_b, const Vec2T<Float> center,
                                  Float radius);
}  // namespace nbody::math
