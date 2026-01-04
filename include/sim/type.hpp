
#pragma once

#include <functional>

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// TODO: Implement PointMassT.
template <FloatT Float>
struct PointMassT {
    math::Vec2T<Float> pos{0.0, 0.0};
    Float              mass{1.0};

    std::string to_string() const {
        return std::format("{{pos: {}, mass: {}}}", pos.to_string(), std::to_string(mass));
    }
};

template <FloatT Float>
struct BodyT {
    PointMassT<Float>  pm;
    math::Vec2T<Float> vel{0.0, 0.0};
    math::Vec2T<Float> acc{0.0, 0.0};

    std::string to_string() const {
        return std::format("{{point_mass {}, vel: {}, acc: {}}}", pm.to_string(), vel.to_string(), acc.to_string());
    }
};

using BodyF32 = BodyT<F32>;
using BodyF64 = BodyT<F64>;

template <FloatT Float>
using IntegrateBodyFnT = std::function<void(BodyT<Float>& body, Float dt)>;
};  // namespace nbody::sim
