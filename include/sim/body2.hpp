#pragma once

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody {
template <FloatingPointT Float>
struct Body2T {
    Vec2T<Float> pos{static_cast<Float>(0.0), static_cast<Float>(0.0)};
    Vec2T<Float> vel{static_cast<Float>(0.0), static_cast<Float>(0.0)};
    Vec2T<Float> acc{static_cast<Float>(0.0), static_cast<Float>(0.0)};
    Float        mass{static_cast<Float>(1.0)};
};

using Body2F32 = Body2T<float>;
using Body2F64 = Body2T<double>;

};  // namespace nbody
