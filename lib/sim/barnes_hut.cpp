#include "sim/barnes_hut.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <vector>

#include "base/assert.hpp"
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
std::vector<const typename BarnesHut<Float>::Node*> BarnesHut<Float>::collect_nodes() const {
    std::vector<const Node*> stack;
    std::vector<const Node*> bodies;

    stack.push_back(m_root.get());
    bodies.reserve(m_bodies.size());

    while (!stack.empty()) {
        const Node* current = stack.back();
        stack.pop_back();
        bodies.push_back(current);

        for (USize qid = 0; qid < 4; ++qid) {
            if (current->children[qid]) {
                stack.push_back(current->children[qid].get());
            }
        }
    }

    return bodies;
}

template <FloatT Float>
void BarnesHut<Float>::impl_create_root() {
    if (m_bodies.empty()) {
        m_root = Node::make_ptr_region(Vec2::make_zero(), 0.0);
        return;
    }

    auto [center, radius] = impl_root_node_center_radius();

    m_root = Node::make_ptr_region(center, radius);
}

template <FloatT Float>
void BarnesHut<Float>::impl_construct_tree() {
    ASSERT(m_root, "Root is not initlialized");

    for (const Body& body : m_bodies) {
        m_root->insert_point_mass(body.pm);
    }
}

template <FloatT Float>
void BarnesHut<Float>::impl_apply_gravity() {
    ASSERT(m_root, "Root is not initlialized");

    for (Body& body : m_bodies) {
        m_root->apply_gravity_body(body, m_g, m_softening, m_theta);
    }
}

template <FloatT Float>
std::tuple<typename BarnesHut<Float>::Vec2, Float>
BarnesHut<Float>::impl_root_node_center_radius() {
    Float max_x = std::numeric_limits<Float>::lowest();
    Float max_y = std::numeric_limits<Float>::lowest();
    Float min_x = std::numeric_limits<Float>::max();
    Float min_y = std::numeric_limits<Float>::max();

    for (const Body& body : m_bodies) {
        max_x = std::max(max_x, body.pm.pos.x);
        max_y = std::max(max_y, body.pm.pos.y);
        min_x = std::min(min_x, body.pm.pos.x);
        min_y = std::min(min_y, body.pm.pos.y);
    }

    Float width  = max_x - min_x;
    Float height = max_y - min_y;

    Float radius = std::max(width, height) / 2.0;
    radius       = std::max(radius, M_MIN_ROOT_QUAD_RADIUS);

    Vec2 center = Vec2{
        static_cast<Float>(min_x + width / 2.0),
        static_cast<Float>(min_y + height / 2.0),
    };

    return std::make_tuple(center, radius);
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_empty(
    const Vec2& qc, Float qr) {
    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->quad_center          = qc;
    node->quad_radius          = qr;
    node->kind                 = NodeKind::EMPTY;

    return node;
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_region(
    const Vec2& qc, Float qr) {
    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->quad_center          = qc;
    node->quad_radius          = qr;
    node->kind                 = NodeKind::REGION;

    return node;
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_leaf(
    const Vec2& qc, Float qr, const PointMass& pm) {
    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->quad_center          = qc;
    node->quad_radius          = qr;
    node->center               = pm.pos;
    node->mass                 = pm.mass;
    node->kind                 = NodeKind::LEAF;

    return node;
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
std::string BarnesHut<Float>::Node::to_string() const {
    // TODO: Implement.
    return "TODO";
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
        weighted_sum = weighted_sum.add(children[q]->center.scale(children[q]->mass));
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

template <FloatT Float>
void BarnesHut<Float>::Node::impl_apply_gravity_body_source(Body& body, const PointMass& pm,
                                                            Float g, Float softening) const {
    Vec2  delta   = pm.pos.sub(body.pm.pos);
    Float r2_soft = delta.length_sq() + (softening * softening);

    if (r2_soft <= math::impl::default_epsilon<Float>()) {
        return;
    }

    Float inv_r3              = 1.0 / (std::sqrt(r2_soft) * r2_soft);
    Vec2  source_contribution = delta.scale(g * pm.mass * inv_r3);
    body.acc                  = body.acc.add(source_contribution);
}

template class BarnesHut<F32>;
template class BarnesHut<F64>;

}  // namespace nbody::sim
