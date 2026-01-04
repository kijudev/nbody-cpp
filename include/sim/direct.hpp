#pragma once

#include <cmath>
#include <span>

#include "base/type.hpp"
#include "type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class SimDirect {
   public:
    using Vec2        = math::Vec2T<Float>;
    using Body        = BodyT<Float>;
    using Layout      = std::vector<Body>;
    using IntegrateFn = IntegrateFnT<Float>;

    struct Config {
        Layout      bodies;
        IntegrateFn integrate_fn;
        Float       g;
        Float       softening;
    };

    SimDirect(Config config)
        : m_bodies(std::move(config.bodies)),
          m_integrate(std::move(config.integrate_fn)),
          m_g(config.g),
          m_softening(config.softening) {}

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const { return m_bodies; }
    void add_body(const Body& body) { m_bodies.push_back(body); }

    void step(Float dt) {
        for (Body& body : m_bodies) {
            body.acc = Vec2::make_zero();
        }

        apply_gravity();

        for (Body& body : m_bodies) {
            m_integrate(body, dt);
        }
    }

   private:
    Layout m_bodies{};

    const IntegrateFn m_integrate{};
    const Float       m_g{};
    const Float       m_softening{};

    void apply_gravity() {
        for (USize i = 0; i < m_bodies.size(); ++i) {
            for (USize j = i + 1; j < m_bodies.size(); ++j) {
                apply_gravity_body_pair(m_bodies[i], m_bodies[j]);
            }
        }
    }

    void apply_gravity_body_pair(Body& a, Body& b) {
        Vec2  delta     = b.pm.pos.sub(a.pm.pos);
        Float r2_soft   = delta.length_sq() + (m_softening * m_softening);
        Float inv_r3    = 1.0 / (std::sqrt(r2_soft) * r2_soft);
        Vec2  a_contrib = delta.scale(m_g * b.pm.mass * inv_r3);
        Vec2  b_contrib = delta.scale(m_g * a.pm.mass * inv_r3);

        a.acc = a.acc.add(a_contrib);
        b.acc = b.acc.sub(b_contrib);
    }
};
}  // namespace nbody::sim
