
#pragma once

#include <functional>

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody2 {

// TODO: Implement PointMassT.
template <FloatT Float>
struct PointMassT {
    math::Vec2T<Float> pos{0.0, 0.0};
    Float              mass{1.0};
};

template <FloatT Float>
struct BodyT {
    math::Vec2T<Float> pos{0.0, 0.0};
    math::Vec2T<Float> vel{0.0, 0.0};
    math::Vec2T<Float> acc{0.0, 0.0};
    Float              mass{1.0};

    std::string fmt() const {
        return std::format("{{pos: {}, vel: {}, acc: {}, mass: {}}}", pos.fmt(), vel.fmt(),
                           acc.fmt(), std::to_string(mass));
    }
};

template <FloatT Float>
using IntegrateFn = std::function<void(BodyT<Float>& body, Float dt)>;
};  // namespace nbody2
