#include "sim/direct.hpp"

#include <algorithm>
#include <cmath>

#include "base/parallel.hpp"
#include "base/type.hpp"

namespace nbody::sim {
using namespace nbody::base;
using namespace nbody::base::type;

template <FloatT Float>
Direct<Float>::Direct(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_g(config.g),
      m_softening(config.softening),
      m_parallel(config.parallel),
      m_integrate_fn(config.integrate_fn) {}

template <FloatT Float>
void Direct<Float>::step(Float dt) {
    if (m_parallel) {
        parallel_for_each(m_bodies.begin(), m_bodies.end(),
                          [](Body& body) { body.acc = Vec2::make_zero(); });
    } else {
        for (Body& body : m_bodies) {
            body.acc = Vec2::make_zero();
        }
    }

    impl_compute_acc();

    if (m_parallel) {
        parallel_for_each(m_bodies.begin(), m_bodies.end(),
                          [this, dt](Body& body) { m_integrate_fn(body, dt); });
    } else {
        for (Body& body : m_bodies) {
            m_integrate_fn(body, dt);
        }
    }
}

template <FloatT Float>
void Direct<Float>::insert_body(Body&& body) {
    m_bodies.push_back(std::move(body));
}

template <FloatT Float>
std::span<const typename Direct<Float>::Body, std::dynamic_extent> Direct<Float>::bodies() const {
    return m_bodies;
}

template <FloatT Float>
void Direct<Float>::impl_compute_acc() {
    if (m_parallel) {
        impl_compute_acc_par();
    } else {
        impl_compute_acc_seq();
    }
}

template <FloatT Float>
void Direct<Float>::impl_compute_acc_seq() {
    const Float softening_sq = m_softening * m_softening;
    const USize n            = m_bodies.size();

    for (USize i = 0; i < n; ++i) {
        Vec2 acc = Vec2::make_zero();

        for (USize j = 0; j < n; ++j) {
            if (i == j) {
                continue;
            }

            const Vec2  delta   = m_bodies[j].pos.sub(m_bodies[i].pos);
            const Float dist_sq = delta.length_sq();

            const Float denominator = (dist_sq + softening_sq) * std::sqrt(dist_sq + softening_sq);
            const Float factor      = m_g * m_bodies[j].mass / denominator;

            acc.x += factor * delta.x;
            acc.y += factor * delta.y;
        }

        m_bodies[i].acc = m_bodies[i].acc.add(acc);
    }
}

template <FloatT Float>
void Direct<Float>::impl_compute_acc_par() {
    const Float softening_sq = m_softening * m_softening;
    const USize n            = m_bodies.size();

    parallel_for_each(m_bodies.begin(), m_bodies.end(), [this, softening_sq, n](Body& body_i) {
        Vec2 acc = Vec2::make_zero();

        for (USize j = 0; j < n; ++j) {
            const Body& body_j = m_bodies[j];

            if (body_i.pos.is_approx_equal(body_j.pos) &&
                std::abs(body_i.mass - body_j.mass) < static_cast<Float>(1e-10)) {
                continue;
            }

            const Vec2  delta   = body_j.pos.sub(body_i.pos);
            const Float dist_sq = delta.length_sq();

            const Float denominator = (dist_sq + softening_sq) * std::sqrt(dist_sq + softening_sq);
            const Float factor      = m_g * body_j.mass / denominator;

            acc.x += factor * delta.x;
            acc.y += factor * delta.y;
        }

        body_i.acc = body_i.acc.add(acc);
    });
}

template class Direct<F32>;
template class Direct<F64>;

}  // namespace nbody::sim
