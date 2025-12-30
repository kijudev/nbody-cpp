#pragma once

#include <cmath>

#include "base/type.hpp"
#include "type.hpp"

namespace nbody {
template <FloatT Float>
class Sim2Direct {
   public:
    using Vec2        = Vec2T<Float>;
    using Body        = Body2T<Float>;
    using Layout      = std::vector<Body>;
    using IntegrateFn = Integrate2Fn<Float>;

    Sim2Direct(Layout bodies, IntegrateFn integrator, Float g, Float softening)
        : m_bodies(std::move(bodies)),
          m_integrate(std::move(integrator)),
          m_g(g),
          m_softening(softening) {}

    [[nodiscard]] std::span<const Body> bodies() const { return m_bodies; }
    void                                add_body(const Body& body) { m_bodies.push_back(body); }

    void step(Float dt) {
        for (Body& body : m_bodies) {
            body.acc = Vec2::zero();
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
        for (size_t i = 0; i < m_bodies.size(); ++i) {
            for (size_t j = i + 1; j < m_bodies.size(); ++j) {
                apply_gravity_body_pair(m_bodies[i], m_bodies[j]);
            }
        }
    }

    void apply_gravity_body_pair(Body& a, Body& b) {
        Vec2  delta     = b.pos.sub(a.pos);
        Float r2_soft   = delta.length_sq() + (m_softening * m_softening);
        Float inv_r3    = 1.0 / (std::sqrt(r2_soft) * r2_soft);
        Vec2  a_contrib = delta.scale(m_g * b.mass * inv_r3);
        Vec2  b_contrib = delta.scale(m_g * a.mass * inv_r3);

        a.acc = a.acc.add(a_contrib);
        b.acc = b.acc.sub(b_contrib);
    }
};
}  // namespace nbody
