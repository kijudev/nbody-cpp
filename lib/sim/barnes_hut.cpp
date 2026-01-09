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
#include "sim/barnes_hut.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
BarnesHut<Float>::BarnesHut(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_integrate(std::move(config.integrate_fn)),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta),
      m_depth(config.depth) {}

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
    // EXPLANATION:
    // (1) Set all the previously updated accelerations to 0.
    // (2) Create the root of the Quad Tree.
    // (3) Construct the Quad Tree.
    // (4) Apply the gravitaional force to all the bodies (calculate the aproximate acceleration).
    // (5) Integrate the gravitational force (update the velocity and position).

    for (Body& body : m_bodies) {
        body.acc = Vec2::make_zero();
    }

    impl_create_root();
    impl_construct_tree();
    impl_apply_gravity();

    for (Body& body : m_bodies) {
        m_integrate(body, dt);
    }
}

template <FloatT Float>
std::vector<const typename BarnesHut<Float>::Node*> BarnesHut<Float>::collect_nodes() const {
    std::vector<const Node*> stack;
    std::vector<const Node*> nodes;

    if (!m_root) return nodes;

    stack.push_back(m_root.get());
    nodes.reserve(m_bodies.size());

    while (!stack.empty()) {
        const Node* current = stack.back();
        stack.pop_back();

        if (!current) continue;
        nodes.push_back(current);

        for (USize qid = 0; qid < 4; ++qid) {
            if (current->children[qid]) {
                stack.push_back(current->children[qid].get());
            }
        }
    }

    return nodes;
}

// --- Quad Tree utils ---

template <FloatT Float>
void BarnesHut<Float>::impl_create_root() {
    // NOTE: if no bodies were provided, create a minimal region root when there are no bodies.
    if (m_bodies.empty()) {
        m_root =
            Node::make_ptr_region(Vec2::make_zero(), static_cast<Float>(M_MIN_ROOT_QUAD_RADIUS));
        return;
    }

    auto [center, radius] = impl_root_node_center_radius();

    // NOTE: Ensure radius is at least the minimum size. The simulation might go unstable if the
    // provided radius is too small.
    radius = std::max(radius, M_MIN_ROOT_QUAD_RADIUS);
    m_root = Node::make_ptr_region(center, radius);
}

template <FloatT Float>
void BarnesHut<Float>::impl_construct_tree() {
    ASSERT(m_root, "Root is not initialized");

    // NOTE: Insert each point mass into the quadtree.
    for (const Body& body : m_bodies) {
        m_root->insert_point_mass(body.pm, 0, m_depth);
    }
}

template <FloatT Float>
void BarnesHut<Float>::impl_apply_gravity() {
    ASSERT(m_root, "Root is not initialized");

    for (Body& body : m_bodies) {
        m_root->apply_gravity_body(body, m_g, m_softening, m_theta);
    }
}

template <FloatT Float>
std::tuple<typename BarnesHut<Float>::Vec2, Float>
BarnesHut<Float>::impl_root_node_center_radius() {
    Float x_max = std::numeric_limits<Float>::lowest();
    Float x_min = std::numeric_limits<Float>::max();
    Float y_max = std::numeric_limits<Float>::lowest();
    Float y_min = std::numeric_limits<Float>::max();

    for (const Body& body : m_bodies) {
        x_max = std::max(x_max, body.pm.pos.x);
        x_min = std::min(x_min, body.pm.pos.x);
        y_max = std::max(y_max, body.pm.pos.y);
        y_min = std::min(y_min, body.pm.pos.y);
    }

    Float width  = x_max - x_min;
    Float height = y_max - y_min;
    Float radius = std::max(width, height) / 2.0;
    radius       = std::max(radius, M_MIN_ROOT_QUAD_RADIUS);

    Vec2 center = Vec2{
        static_cast<Float>(x_min + width / 2.0),
        static_cast<Float>(y_min + height / 2.0),
    };

    return std::make_tuple(center, radius);
}

// --- Quad Tree Node ---

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_empty(
    const Vec2& qc, Float qr) {
    std::unique_ptr<Node> node = std::make_unique<Node>();

    node->quad_center = qc;
    node->quad_radius = qr;
    node->kind        = NodeKind::EMPTY;
    node->mass        = 0.0;
    node->center      = Vec2::make_zero();

    return node;
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_region(
    const Vec2& qc, Float qr) {
    std::unique_ptr<Node> node = std::make_unique<Node>();

    node->quad_center = qc;
    node->quad_radius = qr;
    node->kind        = NodeKind::REGION;
    node->mass        = 0.0;
    node->center      = Vec2::make_zero();

    return node;
}

template <FloatT Float>
std::unique_ptr<typename BarnesHut<Float>::Node> BarnesHut<Float>::Node::make_ptr_leaf(
    const Vec2& qc, Float qr, const PointMass& pm) {
    std::unique_ptr<Node> node = std::make_unique<Node>();

    node->quad_center = qc;
    node->quad_radius = qr;
    node->center      = pm.pos;
    node->mass        = pm.mass;
    node->kind        = NodeKind::LEAF;

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
void BarnesHut<Float>::Node::insert_point_mass(const PointMass& pm, U16 current_depth,
                                               U16 max_depth) {
    // EXPLANATION:
    // (1) If the node is EMPTY, it becomes a LEAF with it's center and mass represeting the point
    // mass of a singular body. Return.
    // (2) If the max recursion depth is reached combine the point mass of the quad with the
    // provided point mass; recalculate the center of mass and mass of the quad. Return.
    // (3) if the is REGION, detemine in which quadrant the provided point mass if ment to fit. If
    // the quadrant is EMPTY, insert the point mass. If it is not, recursively call
    // insert_point_mass on the matching node.
    // (4) If the node is LEAF, subdivide it into 4 quads. Insert the provided point mass and insert
    // the current node as a point mass to their respective nodes.

    // NOTE: If node is EMPTY, it becomes a LEAF with this point mass.
    if (is_empty()) {
        kind   = NodeKind::LEAF;
        center = pm.pos;
        mass   = pm.mass;
        return;
    }

    if (current_depth > max_depth) {
        ASSERT(is_leaf(), "Is not NodeKind::LEAF");

        Float new_mass = mass + pm.mass;
        center.x       = (center.x * mass + pm.pos.x * pm.mass) / new_mass;
        center.y       = (center.y * mass + pm.pos.y * pm.mass) / new_mass;
        mass           = new_mass;
        return;
    }

    if (is_region()) {
        QuadId qid = impl_pos_quad_id(pm.pos);

        if (!children[qid]) {
            const Float child_qr = quad_radius / 2.0;
            const Vec2  child_qc = impl_quad_id_center(qid);
            children[qid]        = make_ptr_leaf(child_qc, child_qr, pm);
        } else {
            children[qid]->insert_point_mass(pm, current_depth + 1, max_depth);
        }

        self_recompute_com_mass();
    } else if (is_leaf()) {
        PointMass self_pm = self_as_point_mass();
        kind              = NodeKind::REGION;

        const Float child_qr = quad_radius / 2.0;
        for (USize q = 0; q < 4; ++q) {
            Vec2 child_qc = impl_quad_id_center(q);
            children[q]   = make_ptr_empty(child_qc, child_qr);
        }

        QuadId self_qid = impl_pos_quad_id(self_pm.pos);
        QuadId new_qid  = impl_pos_quad_id(pm.pos);

        children[self_qid]->insert_point_mass(self_pm, current_depth + 1, max_depth);
        children[new_qid]->insert_point_mass(pm, current_depth + 1, max_depth);

        self_recompute_com_mass();
    }
}

template <FloatT Float>
void BarnesHut<Float>::Node::apply_gravity_body(Body& body, Float g, Float softening,
                                                Float theta) const {
    if (is_empty()) {
        return;
    }

    if (is_leaf()) {
        // NOTE: Directly apply contribution from this point mass.
        impl_apply_gravity_body_source(body, self_as_point_mass(), g, softening);
        return;
    }

    // NOTE: REGION node: decide whether to approximate or recurse; s is the size of the region
    // (side length of the containg quad).
    Float s     = quad_radius * 2.0;
    Vec2  delta = center.sub(body.pm.pos);
    Float dist  = delta.length();
    Float ratio = s / dist;

    if (ratio < theta) {
        impl_apply_gravity_body_source(body, self_as_point_mass(), g, softening);
    } else {
        for (USize q = 0; q < 4; ++q) {
            if (children[q]) {
                children[q]->apply_gravity_body(body, g, softening, theta);
            }
        }
    }
}

template <FloatT Float>
std::string BarnesHut<Float>::Node::to_string() const {
    std::string kind_str = (is_empty() ? "EMPTY" : (is_leaf() ? "LEAF" : "REGION"));
    return "Node{" + kind_str + ", quad_center: " + center.to_string() +
           ", quad_radius: " + std::to_string(quad_radius) + ", mass: " + std::to_string(mass) +
           "}";
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
        if (!children[q]) continue;
        total_mass += children[q]->mass;
        weighted_sum = weighted_sum.add(children[q]->center.scale(children[q]->mass));
    }

    if (total_mass <= math::impl::default_epsilon<Float>()) {
        mass   = 0.0;
        center = Vec2::make_zero();
    } else {
        mass   = total_mass;
        center = weighted_sum.scale(1.0 / total_mass);
    }
}

template <FloatT Float>
BarnesHut<Float>::QuadId BarnesHut<Float>::Node::impl_pos_quad_id(const Vec2& pos) const {
    QuadId qid = 0;

    if (pos.x >= quad_center.x) {
        qid |= 1;
    }

    if (pos.y >= quad_center.y) {
        qid |= 2;
    }

    return qid;
}

template <FloatT Float>
BarnesHut<Float>::Vec2 BarnesHut<Float>::Node::impl_quad_id_center(QuadId qid) const {
    Float half       = quad_radius / 2.0;
    Vec2  new_center = Vec2::make_zero();

    if (qid & 1) {
        new_center.x = quad_center.x + half;
    } else {
        new_center.x = quad_center.x - half;
    }

    if (qid & 2) {
        new_center.y = quad_center.y + half;
    } else {
        new_center.y = quad_center.y - half;
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
