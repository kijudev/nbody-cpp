
#pragma once

#include <functional>

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody2 {
template <FloatT Float>
struct BodyT {
    math::Vec2T<Float> pos{0.0, 0.0};
    math::Vec2T<Float> vel{0.0, 0.0};
    math::Vec2T<Float> acc{0.0, 0.0};
    Float              mass{1.0};

    BodyT() = default;
    BodyT(math::Vec2T<Float> pos, math::Vec2T<Float> vel, math::Vec2T<Float> acc, Float mass)
        : pos(pos), vel(vel), acc(acc), mass(mass) {}

    std::string fmt() const {
        return std::format("{{pos: {}, vel: {}, acc: {}, mass: {}}}", pos.fmt(), vel.fmt(),
                           acc.fmt(), std::to_string(mass));
    }
};

template <FloatT Float>
using IntegrateFn = std::function<void(BodyT<Float>& body, Float dt)>;
};  // namespace nbody2
