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

    struct Config {
        Layout          bodies{};
        IntegrateBodyFn integrate_fn = integrate_body_euler<Float>;
        Float           g            = G_TOY;
        Float           softening    = SOFTENING_TOY;
    };

   public:
    Direct(const Config& config);

   public:
    void                                                     step(Float dt);
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;
    void                                                     insert_body(Body&& body);

   private:
    Layout m_bodies{};

    const IntegrateBodyFn m_integrate_body{integrate_body_euler<Float>};
    const Float           m_g{G_TOY};
    const Float           m_softening{SOFTENING_TOY};

   private:
    void impl_apply_gravity();
    void impl_apply_gravity_body_pair(Body& a, Body& b);
};
}  // namespace nbody::sim
