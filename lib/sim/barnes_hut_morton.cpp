#include "sim/barnes_hut_morton.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>
#include <utility>

#include "base/assert.hpp"
#include "base/type.hpp"
#include "math/morton.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float, math::MortonCodeT MortonCode>
BarnesHutMorton<Float, MortonCode>::BarnesHutMorton(const Config& config)
    : m_integrate(config.integrate_fn),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta),
      m_bodies(std::move(config.bodies)),
      m_bodies_lookup(m_bodies.size(), false) {}

template <FloatT Float, math::MortonCodeT MortonCode>
std::span<const typename BarnesHutMorton<Float, MortonCode>::Body, std::dynamic_extent>
BarnesHutMorton<Float, MortonCode>::bodies() const {
    return m_bodies;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::insert_body(Body&& body) {
    m_bodies.emplace_back(body);
    m_bodies_lookup.emplace_back(false);
    return;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::step(Float dt) {
    impl_reset_acceleration_all();
    impl_create_tree();
    impl_compute_acceleration_all();
    impl_integrate_all(dt);
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_reset_acceleration_all() {
    for (USize i = 0; i < m_bodies.size(); ++i) {
        if (m_bodies_lookup[i]) continue;
        m_bodies[i].acc = Vec2::make_zero();
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_create_tree() {
    impl_compute_root_center_radius();
    impl_compute_bodies_sorted();
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_compute_acceleration_all() {
    // EXPLANATION:
    // (1) If there are no bodies, return.
    // (2) Clear the nodes vector and reserve enough space for the nodes.
    // (3) Create the root node.
    // (4) Insert each body into the Morton Quad Tree.
    // (5) Compute the mass and center of mass for each node in the tree.
    // (6) For each body, compute the acceleration using the Quad Tree.

    if (m_bodies_sorted.empty()) {
        return;
    }

    m_nodes.clear();
    m_nodes.reserve(m_bodies_sorted.size() * 2);

    Node root{
        .code        = 0,
        .pos         = Vec2::make_zero(),
        .mass        = 0.0,
        .quad_radius = m_root_radius,
        .children    = {NODE_EMPTY, NODE_EMPTY, NODE_EMPTY, NODE_EMPTY},
    };

    m_nodes.push_back(root);

    for (const auto& [body, code] : m_bodies_sorted) {
        impl_insert_body_to_tree(body.pm, code, m_root_radius);
    }

    if (!m_nodes.empty()) {
        impl_compute_node_mass_recursive(NODE_ROOT);
    }

    for (USize i = 0; i < m_bodies.size(); ++i) {
        if (m_bodies_lookup[i]) continue;
        impl_compute_acceleration_body_node(m_bodies[i], NODE_ROOT);
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_insert_body_to_tree(const PointMass& pm,
                                                                  MortonCode       code,
                                                                  Float initial_quad_radius) {
    // EXPLANATION:
    // (1) Start at the root node.
    // (2) Traverse the tree based on the Morton code.
    // (3) If a child node is empty, create a leaf node for the body there.
    // (4) If a child node is a leaf and contains a different body, split the leaf into a region and
    // reinsert both bodies.
    // (5) If a child node is a leaf and contains the same body (same Morton
    // code), merge the bodies by updating the position and mass.
    // (6) If a child node is a region, continue traversing down the tree.
    // (7) If the maximum depth is reached, merge the body into the current node.

    USize current_node_index = NODE_ROOT;
    Float current_quad_size  = initial_quad_radius;
    USize current_depth      = 0;

    while (current_depth < MAX_DEPTH) {
        QuadId current_quad = impl_get_quad_from_morton(code, current_depth);

        if (m_nodes[current_node_index].children[current_quad] == NODE_EMPTY) {
            // NOTE: Create leaf node.
            Float child_quad_radius = current_quad_size / 2.0;
            Node  leaf{
                 .code        = code,
                 .pos         = pm.pos,
                 .mass        = pm.mass,
                 .quad_radius = child_quad_radius,
                 .children    = {NODE_EMPTY, NODE_EMPTY, NODE_EMPTY, NODE_EMPTY},
            };

            USize leaf_idx = m_nodes.size();
            m_nodes.push_back(leaf);
            m_nodes[current_node_index].children[current_quad] = leaf_idx;

            return;
        } else {
            USize child_index = m_nodes[current_node_index].children[current_quad];

            // NOTE: Check if child is a leaf and needs to be split.
            bool child_is_leaf = impl_is_node_leaf(m_nodes[child_index]);

            if (child_is_leaf && m_nodes[child_index].code != code) {
                MortonCode existing_code = m_nodes[child_index].code;
                PointMass  existing_pm{m_nodes[child_index].pos, m_nodes[child_index].mass};

                m_nodes[child_index].pos  = Vec2::make_zero();
                m_nodes[child_index].mass = 0.0;

                // NOTE: Reinsert existing body.
                Float  child_quad_radius = current_quad_size / 2.0;
                QuadId existing_quad = impl_get_quad_from_morton(existing_code, current_depth + 1);
                Node   existing_leaf{
                      .code        = existing_code,
                      .pos         = existing_pm.pos,
                      .mass        = existing_pm.mass,
                      .quad_radius = static_cast<Float>(child_quad_radius / 2.0),
                      .children    = {NODE_EMPTY, NODE_EMPTY, NODE_EMPTY, NODE_EMPTY},
                };

                USize existing_index = m_nodes.size();
                m_nodes.push_back(existing_leaf);
                m_nodes[child_index].children[existing_quad] = existing_index;

                // NOTE: Continue inserting new body.
                current_node_index = child_index;
                current_quad_size  = child_quad_radius;
                current_depth++;
            } else if (child_is_leaf && m_nodes[child_index].code == code) {
                // NOTE: Same position, merge masses.
                Float old_mass = m_nodes[child_index].mass;
                Float new_mass = old_mass + pm.mass;

                m_nodes[child_index].pos.x =
                    (m_nodes[child_index].pos.x * old_mass + pm.pos.x * pm.mass) / new_mass;
                m_nodes[child_index].pos.y =
                    (m_nodes[child_index].pos.y * old_mass + pm.pos.y * pm.mass) / new_mass;
                m_nodes[child_index].mass = new_mass;
                return;
            } else {
                // NOTE: Internal node, continue traversal.
                current_node_index = child_index;
                current_quad_size  = current_quad_size / 2.0;
                ++current_depth;
            }
        }
    }

    // NOTE: Max depth reached, merge with current node.
    if (current_node_index < m_nodes.size()) {
        Float old_mass = m_nodes[current_node_index].mass;
        Float new_mass = old_mass + pm.mass;

        ASSERT(new_mass > 0.0, "Mass has to be greater than 0");

        m_nodes[current_node_index].pos.x =
            (m_nodes[current_node_index].pos.x * old_mass + pm.pos.x * pm.mass) / new_mass;
        m_nodes[current_node_index].pos.y =
            (m_nodes[current_node_index].pos.y * old_mass + pm.pos.y * pm.mass) / new_mass;
        m_nodes[current_node_index].mass = new_mass;
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::QuadId
BarnesHutMorton<Float, MortonCode>::impl_get_quad_from_morton(MortonCode code, USize level) const {
    // NOTE: Extract 2 bits at the specified level.
    USize  shift = (sizeof(MortonCode) * 8 - 2 - level * 2);
    QuadId quad  = (code >> shift) & 0x3;
    return quad;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_compute_node_mass_recursive(USize node_idx) {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");

    Node& node = m_nodes[node_idx];
    if (node.mass < 0.0 || impl_is_node_leaf(node)) {
        return;
    }

    Float total_mass   = 0.0;
    Vec2  weighted_pos = Vec2::make_zero();

    for (QuadId q = 0; q < 4; ++q) {
        USize child_idx = node.children[q];
        if (child_idx != NODE_EMPTY && child_idx < m_nodes.size()) {
            impl_compute_node_mass_recursive(child_idx);

            Float child_mass = m_nodes[child_idx].mass;
            total_mass += child_mass;
            weighted_pos = weighted_pos.add(m_nodes[child_idx].pos.scale(child_mass));
        }
    }

    node.mass = total_mass;
    if (total_mass > 0.0) {
        node.pos = weighted_pos.scale(1.0 / total_mass);
    } else {
        node.pos = Vec2::make_zero();
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_compute_acceleration_body_node(Body& body,
                                                                             USize node_idx) {
    ASSERT(node_idx < m_nodes.size(), "Invalid node index");

    const Node& node = m_nodes[node_idx];

    if (node.mass <= 0.0) {
        return;
    }

    if (impl_is_node_leaf(node)) {
        // NOTE: Skip self-interaction - check if this leaf contains the same body.
        Vec2  delta_leaf = node.pos.sub(body.pm.pos);
        Float dist_leaf  = delta_leaf.length();
        if (dist_leaf > 0.0) {
            impl_compute_acceleration_body_source(body, node.pos, node.mass);
        }
        return;
    }

    Vec2  delta = node.pos.sub(body.pm.pos);
    Float dist  = delta.length();

    if (dist <= 0.0) {
        for (QuadId q = 0; q < 4; ++q) {
            if (node.children[q] != NODE_EMPTY) {
                impl_compute_acceleration_body_node(body, node.children[q]);
            }
        }
        return;
    }

    Float node_size = node.quad_radius * 2.0;
    Float ratio     = node_size / dist;

    if (ratio < m_theta) {
        // NOTE: Far enough, use approximation.
        impl_compute_acceleration_body_source(body, node.pos, node.mass);
    } else {
        // NOTE: Too close, recurse to children.
        for (QuadId q = 0; q < 4; ++q) {
            if (node.children[q] != NODE_EMPTY) {
                impl_compute_acceleration_body_node(body, node.children[q]);
            }
        }
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_compute_acceleration_body_source(
    Body& body, const Vec2& source_pos, Float source_mass) {
    Vec2  delta   = source_pos.sub(body.pm.pos);
    Float r2_soft = delta.length_sq() + (m_softening * m_softening);

    if (r2_soft <= 0.0) {
        return;
    }

    Float inv_r3         = 1.0 / (std::sqrt(r2_soft) * r2_soft);
    Vec2  source_contrib = delta.scale(m_g * source_mass * inv_r3);
    body.acc             = body.acc.add(source_contrib);
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_integrate_all(Float dt) {
    for (USize i = 0; i < m_bodies.size(); ++i) {
        if (m_bodies_lookup[i]) continue;
        m_integrate(m_bodies[i], dt);
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_compute_root_center_radius() {
    if (m_bodies.empty()) {
        m_root_center = Vec2::make_zero();
        m_root_radius = MIN_ROOT_RADIUS;
        return;
    }

    Float x_max = std::numeric_limits<Float>::lowest();
    Float x_min = std::numeric_limits<Float>::max();
    Float y_max = std::numeric_limits<Float>::lowest();
    Float y_min = std::numeric_limits<Float>::max();

    for (USize i = 0; i < m_bodies.size(); ++i) {
        if (m_bodies_lookup[i]) continue;

        const Body& body = m_bodies[i];
        x_max            = std::max(x_max, body.pm.pos.x);
        x_min            = std::min(x_min, body.pm.pos.x);
        y_max            = std::max(y_max, body.pm.pos.y);
        y_min            = std::min(y_min, body.pm.pos.y);
    }

    Float width  = x_max - x_min;
    Float height = y_max - y_min;
    Float radius = std::max(width, height) / 2.0;
    radius       = std::max(radius, MIN_ROOT_RADIUS);

    Vec2 center = Vec2{
        static_cast<Float>(x_min + width / 2.0),
        static_cast<Float>(y_min + height / 2.0),
    };

    m_root_center = center;
    m_root_radius = radius;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::impl_compute_bodies_sorted() {
    m_bodies_sorted.clear();

    if (m_bodies.empty()) {
        return;
    }

    const Float morton_min = -m_root_radius;
    const Float morton_max = m_root_radius;

    for (USize i = 0; i < m_bodies.size(); ++i) {
        if (m_bodies_lookup[i]) continue;

        const Body& body = m_bodies[i];
        // NOTE: Center coordinates before Morton encoding
        Float      rel_x = body.pm.pos.x - m_root_center.x;
        Float      rel_y = body.pm.pos.y - m_root_center.y;
        MortonCode code =
            math::morton_encode2<Float, MortonCode>(rel_x, rel_y, morton_min, morton_max);
        m_bodies_sorted.push_back(std::make_pair(body, code));
    }

    // NOTE: Sort by Morton code; ascending for Z-order traversal.
    std::sort(m_bodies_sorted.begin(), m_bodies_sorted.end(),
              [](const std::pair<Body, MortonCode>& a, const std::pair<Body, MortonCode>& b) {
                  return a.second < b.second;
              });
}

template <FloatT Float, math::MortonCodeT MortonCode>
bool BarnesHutMorton<Float, MortonCode>::impl_is_node_leaf(const Node& node) const {
    for (QuadId q = 0; q < 4; ++q) {
        if (node.children[q] != NODE_EMPTY) {
            return false;
        }
    }

    return true;
}

template class BarnesHutMorton<F32, U32>;
template class BarnesHutMorton<F32, U64>;
template class BarnesHutMorton<F64, U32>;
template class BarnesHutMorton<F64, U64>;
}  // namespace nbody::sim
