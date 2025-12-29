#pragma once

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody {
template <FloatT F>
struct Body2T {
    Vec2T<F> pos{0.0, 0.0};
    Vec2T<F> vel{0.0, 0.0};
    Vec2T<F> acc{0.0, 0.0};
    F        mass{1.0};

    static Body2T<F> from_pos_mass(Vec2T<F> pos, F mass) {
        return {pos, Vec2T<F>::zero(), Vec2T<F>::zero(), mass};
    }
};

using Body2F32 = Body2T<float>;
using Body2F64 = Body2T<double>;

};  // namespace nbody
