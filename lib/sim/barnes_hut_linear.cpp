#include "sim/barnes_hut_linear.hpp"

#include <algorithm>
#include <limits>
#include <stack>
#include <tuple>
#include <vector>

#include "base/assert.hpp"
#include "base/type.hpp"
#include "math/impl.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
BarnesHutLinear<Float>::BarnesHutLinear(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_nodes{},
      m_root_index(static_cast<USize>(-1)),
      m_integrate(std::move(config.integrate_fn)),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta),
      m_depth(config.depth) {
    if (config.reserve_nodes > 0) {
        m_nodes.reserve(config.reserve_nodes);
    }
}

template <FloatT Float>
[[nodiscard]] std::span<const typename BarnesHutLinear<Float>::Body, std::dynamic_extent>
BarnesHutLinear<Float>::bodies() const {
    return m_bodies;
}

template <FloatT Float>
void BarnesHutLinear<Float>::insert_body(Body&& body) {
    m_bodies.emplace_back(std::move(body));
}

template <FloatT Float>
void BarnesHutLinear<Float>::step(Float dt) {
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
std::vector<USize> BarnesHutLinear<Float>::collect_node_indices() const {
    std::vector<USize> out;
    if (m_root_index == static_cast<USize>(-1)) return out;

    out.reserve(m_nodes.size());
    std::stack<USize> stack;
    stack.push(m_root_index);

    while (!stack.empty()) {
        USize idx = stack.top();
        stack.pop();

        if (idx >= m_nodes.size()) continue;
        out.push_back(idx);

        const Node& n = m_nodes[idx];
        for (USize q = 0; q < 4; ++q) {
            USize child = n.children[q];
            if (child != static_cast<USize>(-1)) stack.push(child);
        }
    }

    return out;
}

template <FloatT Float>
void BarnesHutLinear<Float>::impl_create_root() {
    // Reset node storage for this step and prepare a node pool that grows via push_back.
    // We no longer rely on pre-sized vector slots and index-preserving assignments.
    m_nodes.clear();
    m_next_free  = 0;
    m_root_index = static_cast<USize>(-1);

    // Reserve a conservative upper bound to avoid repeated reallocations while
    // building the tree. Use an estimate proportional to the number of bodies.
    // But do NOT call resize() — we will grow with push_back to avoid invalidating
    // references assumptions.
    const USize base_min = 64;
    const USize reserve_estimate =
        (m_bodies.empty() ? base_min : static_cast<USize>(m_bodies.size()) * 8 + base_min);

    if (m_nodes.capacity() < reserve_estimate) {
        m_nodes.reserve(reserve_estimate);
    }
    // Do not pre-size with resize(); use push_back to grow the pool safely.
    // m_next_free will be kept in sync with m_nodes.size() after push_back operations.

    if (m_bodies.empty()) {
        Vec2  zero   = Vec2::make_zero();
        Float min_r  = static_cast<Float>(M_MIN_ROOT_QUAD_RADIUS);
        m_root_index = node_make_region(zero, min_r);
        return;
    }

    auto [center, radius] = impl_root_node_center_radius();
    radius                = std::max(radius, static_cast<Float>(M_MIN_ROOT_QUAD_RADIUS));
    m_root_index          = node_make_region(center, radius);
}

template <FloatT Float>
void BarnesHutLinear<Float>::impl_construct_tree() {
    ASSERT(m_root_index != static_cast<USize>(-1), "Root is not initialized");
    for (const Body& body : m_bodies) {
        node_insert_point_mass(m_root_index, body.pm, 0, m_depth);
    }
}

template <FloatT Float>
void BarnesHutLinear<Float>::impl_apply_gravity() {
    if (m_root_index == static_cast<USize>(-1)) return;

    for (Body& body : m_bodies) {
        node_apply_gravity_at(m_root_index, body, m_g, m_softening, m_theta);
    }
}

template <FloatT Float>
std::tuple<typename BarnesHutLinear<Float>::Vec2, Float>
BarnesHutLinear<Float>::impl_root_node_center_radius() const {
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
    radius       = std::max(radius, static_cast<Float>(M_MIN_ROOT_QUAD_RADIUS));

    Vec2 center = Vec2{
        static_cast<Float>(min_x + width / 2.0),
        static_cast<Float>(min_y + height / 2.0),
    };

    return std::make_tuple(center, radius);
}

template <FloatT Float>
USize BarnesHutLinear<Float>::node_make_region(const Vec2& qc, Float qr) {
    Node n;
    n.quad_center = qc;
    n.quad_radius = qr;
    n.center      = Vec2::make_zero();
    n.mass        = 0.0;
    n.kind        = NodeKind::REGION;
    n.children    = {static_cast<USize>(-1), static_cast<USize>(-1), static_cast<USize>(-1),
                     static_cast<USize>(-1)};
    // Append node to pool and return its index.
    USize idx = static_cast<USize>(m_nodes.size());
    m_nodes.push_back(std::move(n));
    m_next_free = static_cast<USize>(m_nodes.size());
    return idx;
}

template <FloatT Float>
USize BarnesHutLinear<Float>::node_make_empty(const Vec2& qc, Float qr) {
    Node n;
    n.quad_center = qc;
    n.quad_radius = qr;
    n.center      = Vec2::make_zero();
    n.mass        = 0.0;
    n.kind        = NodeKind::EMPTY;
    n.children    = {static_cast<USize>(-1), static_cast<USize>(-1), static_cast<USize>(-1),
                     static_cast<USize>(-1)};
    // Append node to pool and return its index.
    USize idx = static_cast<USize>(m_nodes.size());
    m_nodes.push_back(std::move(n));
    m_next_free = static_cast<USize>(m_nodes.size());
    return idx;
}

template <FloatT Float>
USize BarnesHutLinear<Float>::node_make_leaf(const Vec2& qc, Float qr, const PointMass& pm) {
    Node n;
    n.quad_center = qc;
    n.quad_radius = qr;
    n.center      = pm.pos;
    n.mass        = pm.mass;
    n.kind        = NodeKind::LEAF;
    n.children    = {static_cast<USize>(-1), static_cast<USize>(-1), static_cast<USize>(-1),
                     static_cast<USize>(-1)};
    // Append node to pool and return its index.
    USize idx = static_cast<USize>(m_nodes.size());
    m_nodes.push_back(std::move(n));
    m_next_free = static_cast<USize>(m_nodes.size());
    return idx;
}

template <FloatT Float>
void BarnesHutLinear<Float>::node_insert_point_mass(USize node_idx, const PointMass& pm,
                                                    U16 current_depth, U16 max_depth) {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");

    // Avoid holding references to m_nodes elements across allocations.
    // Access m_nodes by index each time after potential emplace_back to avoid invalidation.

    // If empty node -> become leaf
    if (m_nodes[node_idx].is_empty()) {
        m_nodes[node_idx].kind   = NodeKind::LEAF;
        m_nodes[node_idx].center = pm.pos;
        m_nodes[node_idx].mass   = pm.mass;
        return;
    }

    // If we've exceeded depth, accumulate into this leaf (must be leaf)
    if (current_depth > max_depth) {
        ASSERT(m_nodes[node_idx].is_leaf(), "Is not NodeKind::LEAF");

        Float old_mass   = m_nodes[node_idx].mass;
        Vec2  old_center = m_nodes[node_idx].center;

        Float new_mass = old_mass + pm.mass;
        if (new_mass <= math::impl::default_epsilon<Float>()) {
            m_nodes[node_idx].mass   = 0.0;
            m_nodes[node_idx].center = Vec2::make_zero();
        } else {
            m_nodes[node_idx].center.x = (old_center.x * old_mass + pm.pos.x * pm.mass) / new_mass;
            m_nodes[node_idx].center.y = (old_center.y * old_mass + pm.pos.y * pm.mass) / new_mass;
            m_nodes[node_idx].mass     = new_mass;
        }
        return;
    }

    // REGION case
    if (m_nodes[node_idx].is_region()) {
        QuadId qid = node_impl_pos_quad_id(node_idx, pm.pos);

        USize child_idx = m_nodes[node_idx].children[qid];
        if (child_idx == static_cast<USize>(-1)) {
            // compute child parameters before potentially reallocating the node vector
            Vec2  child_center = node_impl_quad_id_center(node_idx, qid);
            Float child_qr     = m_nodes[node_idx].quad_radius / 2.0;

            // create leaf child (may reallocate m_nodes)
            USize new_child = node_make_leaf(child_center, child_qr, pm);

            // re-acquire node by index and set child
            m_nodes[node_idx].children[qid] = new_child;
        } else {
            // recurse into child (no vector modifications here)
            node_insert_point_mass(child_idx, pm, current_depth + 1, max_depth);
        }

        // recompute center of mass for this node
        node_self_recompute_com_mass(node_idx);
        return;
    }

    // LEAF case: need to split into region and re-insert both masses
    if (m_nodes[node_idx].is_leaf()) {
        // capture previous point mass
        PointMass self_pm = m_nodes[node_idx].self_as_point_mass();

        // convert to region
        m_nodes[node_idx].kind = NodeKind::REGION;

        // prepare child radius
        Float child_qr = m_nodes[node_idx].quad_radius / 2.0;

        // create 4 empty children (each creation may reallocate m_nodes)
        for (USize q = 0; q < 4; ++q) {
            Vec2  child_qc  = node_impl_quad_id_center(node_idx, q);
            USize new_child = node_make_empty(child_qc, child_qr);
            // assign child after creation
            m_nodes[node_idx].children[q] = new_child;
        }

        // determine target child ids for old and new point masses
        QuadId self_qid = node_impl_pos_quad_id(node_idx, self_pm.pos);
        QuadId new_qid  = node_impl_pos_quad_id(node_idx, pm.pos);

        // insert the two point masses into respective children at depth+1
        node_insert_point_mass(m_nodes[node_idx].children[self_qid], self_pm, current_depth + 1,
                               max_depth);
        node_insert_point_mass(m_nodes[node_idx].children[new_qid], pm, current_depth + 1,
                               max_depth);

        // recompute center of mass
        node_self_recompute_com_mass(node_idx);
        return;
    }

    // Shouldn't reach here; defensive return
    return;
}

template <FloatT Float>
void BarnesHutLinear<Float>::node_self_recompute_com_mass(USize node_idx) {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");
    Node& node = m_nodes[node_idx];

    Float total_mass   = 0.0;
    Vec2  weighted_sum = Vec2::make_zero();

    for (USize q = 0; q < 4; ++q) {
        USize child = node.children[q];
        if (child == static_cast<USize>(-1)) continue;
        Node& cn = m_nodes[child];
        total_mass += cn.mass;
        weighted_sum = weighted_sum.add(cn.center.scale(cn.mass));
    }

    if (total_mass <= math::impl::default_epsilon<Float>()) {
        node.mass   = 0.0;
        node.center = Vec2::make_zero();
    } else {
        node.mass   = total_mass;
        node.center = weighted_sum.scale(1.0 / total_mass);
    }
}

template <FloatT Float>
typename BarnesHutLinear<Float>::QuadId BarnesHutLinear<Float>::node_impl_pos_quad_id(
    USize node_idx, const Vec2& pos) const {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");
    const Node& node = m_nodes[node_idx];

    QuadId qid = 0;
    if (pos.x >= node.quad_center.x) qid |= 1;
    if (pos.y >= node.quad_center.y) qid |= 2;
    return qid;
}

template <FloatT Float>
typename BarnesHutLinear<Float>::Vec2 BarnesHutLinear<Float>::node_impl_quad_id_center(
    USize node_idx, QuadId qid) const {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");
    const Node& node = m_nodes[node_idx];

    Float half       = node.quad_radius / 2.0;
    Vec2  new_center = Vec2::make_zero();

    if (qid & 1)
        new_center.x = node.quad_center.x + half;
    else
        new_center.x = node.quad_center.x - half;

    if (qid & 2)
        new_center.y = node.quad_center.y + half;
    else
        new_center.y = node.quad_center.y - half;

    return new_center;
}

template <FloatT Float>
void BarnesHutLinear<Float>::node_impl_apply_gravity_body_source(Body& body, const PointMass& pm,
                                                                 Float g, Float softening) const {
    Vec2  delta   = pm.pos.sub(body.pm.pos);
    Float r2_soft = delta.length_sq() + (softening * softening);

    if (r2_soft <= math::impl::default_epsilon<Float>()) {
        return;
    }

    Float inv_r3         = 1.0 / (std::sqrt(r2_soft) * r2_soft);
    Vec2  source_contrib = delta.scale(g * pm.mass * inv_r3);
    body.acc             = body.acc.add(source_contrib);
}

template <FloatT Float>
void BarnesHutLinear<Float>::node_apply_gravity_at(USize node_idx, Body& body, Float g,
                                                   Float softening, Float theta) const {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");
    const Node& node = m_nodes[node_idx];

    if (node.is_empty()) return;

    if (node.is_leaf()) {
        node_impl_apply_gravity_body_source(body, node.self_as_point_mass(), g, softening);
        return;
    }

    Float s     = node.quad_radius * 2.0;
    Vec2  delta = node.center.sub(body.pm.pos);
    Float dist  = delta.length();

    if (dist <= math::impl::default_epsilon<Float>()) return;

    Float ratio = s / dist;
    if (ratio < theta) {
        node_impl_apply_gravity_body_source(body, node.self_as_point_mass(), g, softening);
    } else {
        for (USize q = 0; q < 4; ++q) {
            USize child = node.children[q];
            if (child != static_cast<USize>(-1)) {
                node_apply_gravity_at(child, body, g, softening, theta);
            }
        }
    }
}

/* explicit template instantiations */
template class BarnesHutLinear<F32>;
template class BarnesHutLinear<F64>;

}  // namespace nbody::sim
