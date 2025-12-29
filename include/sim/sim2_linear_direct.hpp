#pragma once

#include <cmath>

#include "base/type.hpp"
#include "sim/body2.hpp"
#include "sim/integrator2.hpp"

namespace nbody {
template <typename F, typename IntegrateFn>
    requires FloatT<F> && Integrate2FnT<F, IntegrateFn>
class Sim2LinearDirect {
   public:
    using Vec2 = Vec2T<F>;
    using Body = Body2T<F>;

    const F G         = 1.0;
    const F SOFTENING = 0.1;

    void step(F dt) {
        for (Body& body : m_bodies) {
            body.acc = Vec2::zero();
        }

        calc_acc();

        for (Body& body : m_bodies) {
            m_integrate(body, dt);
        }
    }

    Sim2LinearDirect() = default;
    Sim2LinearDirect(const std::vector<Body>& bodies, const IntegrateFn& integrator)
        : m_bodies(bodies), m_integrate(integrator) {}

    [[nodiscard]] const std::vector<Body>& bodies() const noexcept { return m_bodies; }
    [[nodiscard]] std::vector<Body>&       bodies_mut() noexcept { return m_bodies; }

   private:
    std::vector<Body> m_bodies{};
    IntegrateFn       m_integrate{};

    void calc_acc() {
        for (size_t i = 0; i < m_bodies.size(); ++i) {
            Body& a = m_bodies[i];

            for (size_t j = i + 1; j < m_bodies.size(); ++j) {
                Body& b = m_bodies[j];

                apply_gravity_pair(a, b);
            }
        }
    }

    void apply_gravity_pair(Body& a, Body& b) {
        // delta: vector from a to b
        Vec2 delta = b.pos.sub(a.pos);

        // softened squared distance (Plummer softening)
        F r2_soft = delta.length_sq() + (SOFTENING * SOFTENING);

        // inverse r^3 term: 1 / (r2_soft)^(3/2)
        // softening ensures denominator is non-zero
        F inv_r3 = 1.0 / (std::sqrt(r2_soft) * r2_soft);

        // acceleration contributions:
        // a.acc += G * m_b * delta * inv_r3
        // b.acc -= G * m_a * delta * inv_r3
        Vec2 a_contrib = delta.scale(G * b.mass * inv_r3);
        Vec2 b_contrib = delta.scale(G * a.mass * inv_r3);

        a.acc = a.acc.add(a_contrib);
        b.acc = b.acc.sub(b_contrib);
    }
};
}  // namespace nbody
