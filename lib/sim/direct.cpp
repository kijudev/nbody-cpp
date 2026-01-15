#include <algorithm>
#include <cmath>

#include "base/parallel.hpp"
#include "base/type.hpp"
#include "sim/direct.hpp"

namespace nbody::sim {
using namespace nbody::base;
using namespace nbody::base::type;

template <FloatT Float>
Direct<Float>::Direct(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_g(config.g),
      m_softening(config.softening),
      m_parallel(config.parallel),
      m_use_proper_verlet(config.use_proper_verlet),
      m_integrate_fn(config.integrate_fn) {
    if (m_use_proper_verlet) {
        m_old_accelerations.resize(m_bodies.size(), Vec2::make_zero());
    }
}

template <FloatT Float>
void Direct<Float>::step(Float dt) {
    if (m_use_proper_verlet) {
        // NOTE: Proper Velocity Verlet algorithm:
        // (1) x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
        // (2) Compute a(t+dt) from new positions
        // (3) v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt

        const Float half  = 0.5;
        const Float dt_sq = dt * dt;

        if (m_old_accelerations.size() != m_bodies.size()) {
            m_old_accelerations.resize(m_bodies.size(), Vec2::make_zero());
        }

        for (USize i = 0; i < m_bodies.size(); ++i) {
            Body& body             = m_bodies[i];
            m_old_accelerations[i] = body.acc;
            body.pos               = body.pos.add(body.vel.scale(dt))
                           .add(body.acc.scale(half * dt_sq));
        }

        for (Body& body : m_bodies) {
            body.acc = Vec2::make_zero();
        }
        impl_compute_acc();

        for (USize i = 0; i < m_bodies.size(); ++i) {
            Body& body = m_bodies[i];
            body.vel   = body.vel.add(
                m_old_accelerations[i].add(body.acc).scale(half * dt));
        }
    } else {
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
            parallel_for_each(
                m_bodies.begin(), m_bodies.end(),
                [this, dt](Body& body) { m_integrate_fn(body, dt); });
        } else {
            for (Body& body : m_bodies) {
                m_integrate_fn(body, dt);
            }
        }
    }
}

template <FloatT Float>
void Direct<Float>::insert_body(Body&& body) {
    m_bodies.push_back(std::move(body));
    if (m_use_proper_verlet) {
        m_old_accelerations.push_back(Vec2::make_zero());
    }
}

template <FloatT Float>
std::span<const typename Direct<Float>::Body, std::dynamic_extent>
Direct<Float>::bodies() const {
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

            const Float denominator =
                (dist_sq + softening_sq) * std::sqrt(dist_sq + softening_sq);
            const Float factor = m_g * m_bodies[j].mass / denominator;

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

    parallel_for_each(
        m_bodies.begin(), m_bodies.end(),
        [this, softening_sq, n](Body& body_i) {
            Vec2 acc = Vec2::make_zero();

            for (USize j = 0; j < n; ++j) {
                const Body& body_j = m_bodies[j];

                if (body_i.pos.is_approx_equal(body_j.pos) &&
                    std::abs(body_i.mass - body_j.mass) <
                        static_cast<Float>(1e-10)) {
                    continue;
                }

                const Vec2  delta   = body_j.pos.sub(body_i.pos);
                const Float dist_sq = delta.length_sq();

                const Float denominator = (dist_sq + softening_sq) *
                                          std::sqrt(dist_sq + softening_sq);
                const Float factor = m_g * body_j.mass / denominator;

                acc.x += factor * delta.x;
                acc.y += factor * delta.y;
            }

            body_i.acc = body_i.acc.add(acc);
        });
}

template <FloatT Float>
void Direct<Float>::compute_initial_accelerations() {
    for (Body& body : m_bodies) {
        body.acc = Vec2::make_zero();
    }

    impl_compute_acc();
}

template class Direct<F32>;
template class Direct<F64>;

}  // namespace nbody::sim
