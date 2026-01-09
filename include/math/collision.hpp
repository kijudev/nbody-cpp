#include "base/type.hpp"

namespace nbody::math {
using namespace nbody::base::type;

template <SignedNumberT Number>
bool check_collision2_rect_point(Number rx, Number ry, Number rwidth, Number rheight, Number px,
                                 Number py);

template <SignedNumberT Number>
bool check_collision2_rect_circle(Number rx, Number ry, Number rwidth, Number rheight, Number cx,
                                  Number cy, Number cr);
}  // namespace nbody::math
