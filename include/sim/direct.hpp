#pragma once

#include <cmath>
#include <span>

#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class Direct {
   public:
    using Vec2            = math::Vec2T<Float>;
    using Body            = BodyT<Float>;
    using Layout          = std::vector<Body>;
    using IntegrateBodyFn = IntegrateBodyFnT<Float>;

    static constexpr Float DEAFULT_G         = sim::scale_toy::G;
    static constexpr Float DEAFULT_SOFTENING = sim::scale_toy::SOFTENING;

    struct Config {
        Layout          bodies{};
        IntegrateBodyFn integrate_fn = integrate_body_euler<Float>;
        Float           g            = DEAFULT_G;
        Float           softening    = DEAFULT_SOFTENING;
    };

   public:
    Direct(const Config& config);

    void                                                     step(Float dt);
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;
    void                                                     insert_body(Body&& body);

   private:
    Layout m_bodies{};

    const IntegrateBodyFn m_integrate_body{integrate_body_euler<Float>};
    const Float           m_g{DEAFULT_G};
    const Float           m_softening{DEAFULT_SOFTENING};

    void impl_apply_gravity();
    void impl_apply_gravity_body_pair(Body& a, Body& b);
};
}  // namespace nbody::sim
