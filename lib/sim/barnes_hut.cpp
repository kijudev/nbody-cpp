#include "sim/barnes_hut.hpp"

#include <memory>
#include <span>

#include "base/type.hpp"
#include "math/impl.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
BarnesHut<Float>::BarnesHut(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_integrate(std::move(config.integrate_fn)),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta) {}

template <FloatT Float>
[[nodiscard]] std::span<const typename BarnesHut<Float>::Body, std::dynamic_extent>
BarnesHut<Float>::bodies() const {
    return m_bodies;
}

template <FloatT Float>
void BarnesHut<Float>::insert_body(Body&& body) {
    m_bodies.emplace_back(std::move(body));
}

template <FloatT Float>
void BarnesHut<Float>::step(Float dt) {
    for (Body& body : m_bodies) {
        body.acc = Vec2::make_zero();
    }

    impl_construct_tree();
    impl_apply_gravity();

    for (Body& body : m_bodies) {
        m_integrate(body, dt);
    }
}

template <FloatT Float>
BarnesHut<Float>::Node::Node(const Vec2& quad_center_, Float quad_radius_, const Vec2& center_,
                             Float mass_, NodeKind kind_)
    : quad_center(quad_center_),
      quad_radius(quad_radius_),
      center(center_),
      mass(mass_),
      kind(kind_),
      children{nullptr, nullptr, nullptr, nullptr} {}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_empty(
    const Vec2& qc, Float qr) {
    return std::make_unique<Node>(qc, qr, {0.0, 0.0}, 0.0, NodeKind::EMPTY);
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_region(
    const Vec2& qc, Float qr) {
    return std::make_unique<Node>(qc, qr, {0.0, 0.0}, 0.0, NodeKind::REGION);
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_leaf(
    const Vec2& qc, Float qr, const PointMass& pm) {
    return std::make_unique<Node>(qc, qr, pm.pos, pm.mass, NodeKind::LEAF);
}

template <FloatT Float>
bool BarnesHut<Float>::Node::is_empty() const {
    return kind == NodeKind::EMPTY;
}

template <FloatT Float>
bool BarnesHut<Float>::Node::is_region() const {
    return kind == NodeKind::REGION;
}

template <FloatT Float>
bool BarnesHut<Float>::Node::is_leaf() const {
    return kind == NodeKind::LEAF;
}

template <FloatT Float>
void BarnesHut<Float>::Node::insert_point_mass(const PointMass& pm) {
    if (is_empty()) {
        kind   = NodeKind::LEAF;
        center = pm.pos;
        mass   = pm.mass;
    } else if (is_region()) {
        QuadId qid = impl_pos_quad_id(pm.pos);

        if (!children[qid]) {
            children[qid] = make_ptr_leaf(impl_quad_id_center(qid), quad_radius / 2.0, pm);
        } else {
            children[qid]->insert_point_mass(pm);
        }

        self_recompute_com_mass();
    } else {
        PointMass self_pm = self_as_point_mass();
        kind              = NodeKind::REGION;

        for (USize q = 0; q < 4; ++q) {
            children[q] = make_ptr_empty(impl_quad_id_center(q), quad_radius / 2.0);
        }

        QuadId self_qid = impl_pos_quad_id(self_pm.pos);
        children[self_qid]->insert_point_mass(self_pm);

        QuadId new_qid = impl_pos_quad_id(pm.pos);
        children[new_qid]->insert_point_mass(pm);

        self_recompute_com_mass();
    }
}

template <FloatT Float>
void BarnesHut<Float>::Node::apply_gravity_body(Body& body, Float g, Float softening,
                                                Float theta) const {
    if (is_leaf()) {
        impl_apply_gravity_body_source(body, self_as_point_mass(), g, softening);
        return;
    }

    Float s     = quad_radius * 2.0;
    Vec2  delta = center.sub(body.pm.pos);
    Float dist  = delta.length();

    // Return if the distance is too small for accurate gravitational acceleration.
    if (dist <= math::impl::default_epsilon<Float>()) {
        return;
    }

    Float ratio = s / dist;

    if (ratio < theta) {
        impl_apply_gravity_body_source(body, self_as_point_mass(), g, softening);
    } else {
        for (USize q = 0; q < 4; ++q) {
            children[q]->apply_gravity_body(body, g, softening, theta);
        }
    }
}

template <FloatT Float>
BarnesHut<Float>::PointMass BarnesHut<Float>::Node::self_as_point_mass() const {
    return PointMass{
        .pos  = center,
        .mass = mass,
    };
}

template <FloatT Float>
void BarnesHut<Float>::Node::self_recompute_com_mass() {
    Float total_mass   = 0.0;
    Vec2  weighted_sum = Vec2::make_zero();

    for (USize q = 0; q < 4; ++q) {
        total_mass += children[q]->mass;
        weighted_sum = weighted_sum.add(children[q]->center.mul(children[q]->mass));
    }

    mass   = total_mass;
    center = weighted_sum.scale(1.0 / total_mass);
}

template <FloatT Float>
BarnesHut<Float>::QuadId BarnesHut<Float>::Node::impl_pos_quad_id(const Vec2& pos) const {
    QuadId qid = 0;

    if (pos.x > center.x) {
        qid |= 1;
    }

    if (pos.y > center.y) {
        qid |= 2;
    }

    return qid;
}

template <FloatT Float>
BarnesHut<Float>::Vec2 BarnesHut<Float>::Node::impl_quad_id_center(QuadId qid) const {
    Float half       = quad_radius / 2.0;
    Vec2  new_center = Vec2::make_zero();

    if (qid & 1) {
        new_center.x += half;
    } else {
        new_center.x -= half;
    }

    if (qid & 2) {
        new_center.y += half;
    } else {
        new_center.y -= half;
    }

    return new_center;
}

}  // namespace nbody::sim
