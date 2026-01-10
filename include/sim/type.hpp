
#pragma once

#include <functional>

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
struct BodyT {
    math::Vec2T<Float> pos{0.0, 0.0};
    math::Vec2T<Float> vel{0.0, 0.0};
    math::Vec2T<Float> acc{0.0, 0.0};
    Float              mass{0.0};
};

using BodyF32 = BodyT<F32>;
using BodyF64 = BodyT<F64>;

template <FloatT Float>
using IntegrateBodyFnT = std::function<void(BodyT<Float>& body, Float dt)>;

template <FloatT Float>
class SimInterface {
   public:
    virtual ~SimInterface() = default;

    virtual void step(Float dt) = 0;

    virtual std::span<const BodyT<Float>, std::dynamic_extent> bodies() const = 0;
};
};  // namespace nbody::sim
