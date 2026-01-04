#include "sim/direct.hpp"

#include <algorithm>

#include "base/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
Direct<Float>::Direct(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_integrate_body(std::move(config.integrate_fn)),
      m_g(config.g),
      m_softening(config.softening) {}

template <FloatT Float>
void Direct<Float>::step(Float dt) {
    for (Body& body : m_bodies) {
        body.acc = Vec2::make_zero();
    }

    impl_apply_gravity();

    for (Body& body : m_bodies) {
        m_integrate_body(body, dt);
    }
}

template <FloatT Float>
[[nodiscard]] std::span<const typename Direct<Float>::Body, std::dynamic_extent>
Direct<Float>::bodies() const {
    return m_bodies;
}

template <FloatT Float>
void Direct<Float>::insert_body(Body&& body) {
    m_bodies.emplace_back(std::move(body));
}

template <FloatT Float>
void Direct<Float>::impl_apply_gravity() {
    for (USize i = 0; i < m_bodies.size(); ++i) {
        for (USize j = i + 1; j < m_bodies.size(); ++j) {
            impl_apply_gravity_body_pair(m_bodies[i], m_bodies[j]);
        }
    }
}

template <FloatT Float>
void Direct<Float>::impl_apply_gravity_body_pair(Body& a, Body& b) {
    Vec2  delta     = b.pm.pos.sub(a.pm.pos);
    Float r2_soft   = delta.length_sq() + (m_softening * m_softening);
    Float inv_r3    = 1.0 / (std::sqrt(r2_soft) * r2_soft);
    Vec2  a_contrib = delta.scale(m_g * b.pm.mass * inv_r3);
    Vec2  b_contrib = delta.scale(m_g * a.pm.mass * inv_r3);

    a.acc = a.acc.add(a_contrib);
    b.acc = b.acc.sub(b_contrib);
}

template class Direct<F32>;
template class Direct<F64>;

}  // namespace nbody::sim
