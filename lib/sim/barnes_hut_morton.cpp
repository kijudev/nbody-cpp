#include <algorithm>
#include <cmath>

#include "base/parallel.hpp"
#include "base/radix.hpp"
#include "math/morton.hpp"
#include "sim/barnes_hut_morton.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float, math::MortonCodeT MortonCode>
BarnesHutMorton<Float, MortonCode>::BarnesHutMorton(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta),
      m_parallel(config.parallel),
      m_integrate_fn(config.integrate_fn) {
    m_codes.reserve(m_bodies.size());
    m_nodes.reserve(m_bodies.size() * 2);
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::step(Float dt) {
    // NOTE: Reset accelerations.
    if (m_parallel) {
        base::parallel_for_each(
            m_bodies.begin(), m_bodies.end(),
            [](Body& body) { body.acc = Vec2::make_zero(); });
    } else {
        for (Body& body : m_bodies) {
            body.acc = Vec2::make_zero();
        }
    }

    // NOTE: Build the tree from current body positions.
    compute_extent();
    compute_codes();
    sort_codes();
    build_tree();

    // NOTE: Compute accelerations using tree traversal.
    if (m_parallel) {
        base::parallel_for_each(m_bodies.begin(), m_bodies.end(),
                                [this, idx = USize{0}](Body& body) mutable {
                                    body.acc =
                                        compute_acceleration(body.pos, idx);
                                    ++idx;
                                });
    } else {
        for (USize i = 0; i < m_bodies.size(); ++i) {
            m_bodies[i].acc = compute_acceleration(m_bodies[i].pos, i);
        }
    }

    // NOTE: Integrate bodies.
    if (m_parallel) {
        base::parallel_for_each(
            m_bodies.begin(), m_bodies.end(),
            [this, dt](Body& body) { m_integrate_fn(body, dt); });
    } else {
        for (Body& body : m_bodies) {
            m_integrate_fn(body, dt);
        }
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::insert_body(Body&& body) {
    m_bodies.push_back(std::move(body));
}

template <FloatT Float, math::MortonCodeT MortonCode>
std::span<const typename BarnesHutMorton<Float, MortonCode>::Body,
          std::dynamic_extent>
BarnesHutMorton<Float, MortonCode>::bodies() const {
    return m_bodies;
}

template <FloatT Float, math::MortonCodeT MortonCode>
const std::vector<typename BarnesHutMorton<Float, MortonCode>::Node>&
BarnesHutMorton<Float, MortonCode>::nodes() const {
    return m_nodes;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::compute_extent() {
    if (m_bodies.empty()) {
        m_extent_min = 0.0;
        m_extent_max = 1.0;
        m_root_size  = 1.0;
        return;
    }

    Float x_min = std::numeric_limits<Float>::max();
    Float x_max = std::numeric_limits<Float>::lowest();
    Float y_min = std::numeric_limits<Float>::max();
    Float y_max = std::numeric_limits<Float>::lowest();

    for (const Body& body : m_bodies) {
        x_min = std::min(x_min, body.pos.x);
        x_max = std::max(x_max, body.pos.x);
        y_min = std::min(y_min, body.pos.y);
        y_max = std::max(y_max, body.pos.y);
    }

    // NOTE: Use square bounding box for Morton encoding.
    const Float range_x = x_max - x_min;
    const Float range_y = y_max - y_min;
    const Float range   = std::max(range_x, range_y);

    // NOTE: Add small padding to avoid edge cases.
    const Float padding =
        range * static_cast<Float>(0.01) + static_cast<Float>(1e-10);

    m_extent_min = std::min(x_min, y_min) - padding;
    m_extent_max = m_extent_min + range + 2 * padding;
    m_root_size  = m_extent_max - m_extent_min;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::compute_codes() {
    m_codes.clear();
    m_codes.reserve(m_bodies.size());

    for (USize i = 0; i < m_bodies.size(); ++i) {
        m_codes.push_back(BodyCode{
            .index = i,
            .code  = math::morton_encode2<Float, MortonCode>(
                m_bodies[i].pos.x, m_bodies[i].pos.y, m_extent_min,
                m_extent_max),
        });
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::sort_codes() {
    base::radix_sort(m_codes.begin(), m_codes.end(),
                     [](const BodyCode& bc) { return bc.code; });
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::build_tree() {
    m_nodes.clear();

    if (m_bodies.empty()) {
        return;
    }

    // NOTE: Create leaf nodes from sorted codes.
    build_leaves();

    if (m_nodes.empty()) {
        return;
    }

    if (m_nodes.size() == 1) {
        return;
    }

    // NOTE: Build internal nodes level by level, from fine to coarse.
    // We track which nodes at each level need to be grouped into parents.
    USize children_start = 0;
    USize children_end   = m_nodes.size();

    for (USize level = MAX_DEPTH; level > 0; --level) {
        auto [new_start, new_end] =
            build_level(level - 1, children_start, children_end);

        if (new_start < new_end) {
            if (level == 1 && (new_end - new_start) == 1) {
                break;
            }
            children_start = new_start;
            children_end   = new_end;
        }
    }

    propagate_com_up();
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::build_leaves() {
    // NOTE: Create leaf nodes from sorted body codes.
    // Bodies with identical Morton codes are merged into a single leaf.

    if (m_codes.empty()) {
        return;
    }

    MortonCode current_code = m_codes[0].code;
    Vec2       current_com  = m_bodies[m_codes[0].index].pos;
    Float      current_mass = m_bodies[m_codes[0].index].mass;
    USize      current_body = m_codes[0].index;

    auto flush_leaf = [&]() {
        Node leaf{};
        leaf.prefix     = current_code;
        leaf.level      = MAX_DEPTH;
        leaf.com        = current_com;
        leaf.mass       = current_mass;
        leaf.size       = size_at_level(MAX_DEPTH);
        leaf.is_leaf    = true;
        leaf.body_index = current_body;
        m_nodes.push_back(leaf);
    };

    for (USize i = 1; i < m_codes.size(); ++i) {
        const BodyCode& bc   = m_codes[i];
        const Body&     body = m_bodies[bc.index];

        if (bc.code == current_code) {
            // NOTE: Merge bodies with identical Morton codes.
            const Float new_mass = current_mass + body.mass;
            current_com.x =
                (current_com.x * current_mass + body.pos.x * body.mass) /
                new_mass;
            current_com.y =
                (current_com.y * current_mass + body.pos.y * body.mass) /
                new_mass;
            current_mass = new_mass;
        } else {
            // NOTE: Flush current leaf and start new one.
            flush_leaf();

            current_code = bc.code;
            current_com  = body.pos;
            current_mass = body.mass;
            current_body = bc.index;
        }
    }

    // NOTE: Flush final leaf.
    flush_leaf();
}

template <FloatT Float, math::MortonCodeT MortonCode>
std::pair<USize, USize> BarnesHutMorton<Float, MortonCode>::build_level(
    USize level, USize children_start, USize children_end) {
    // NOTE: Build internal nodes at `level` by grouping children from
    // [children_start, children_end). Children are nodes at level+1 (or deeper)
    // that share the same prefix at `level`.

    if (children_start >= children_end) {
        return {m_nodes.size(), m_nodes.size()};
    }

    const USize nodes_before = m_nodes.size();

    // NOTE: Group children by their prefix at this level.
    MortonCode current_prefix =
        prefix_at_level(m_nodes[children_start].prefix, level);
    USize group_start = children_start;

    auto flush_internal = [&](USize group_end) {
        if (group_start >= group_end) {
            return;
        }

        Node internal{};
        internal.prefix  = current_prefix;
        internal.level   = level;
        internal.com     = Vec2::make_zero();
        internal.mass    = 0.0;
        internal.size    = size_at_level(level);
        internal.is_leaf = false;

        // NOTE: Link children to this internal node and accumulate mass/COM.
        Float total_mass = 0.0;
        Vec2  weighted_com{0.0, 0.0};

        for (USize i = group_start; i < group_end; ++i) {
            Node&    child    = m_nodes[i];
            const U8 quadrant = quadrant_at_level(child.prefix, level);

            // NOTE: Store first child in each quadrant.
            if (internal.children[quadrant] == NODE_INDEX_NULL) {
                internal.children[quadrant] = i;
                child.parent                = m_nodes.size();
            }

            // NOTE: Accumulate for COM calculation.
            weighted_com.x += child.com.x * child.mass;
            weighted_com.y += child.com.y * child.mass;
            total_mass += child.mass;
        }

        // NOTE: Compute center of mass.
        if (total_mass > 0.0) {
            internal.com.x = weighted_com.x / total_mass;
            internal.com.y = weighted_com.y / total_mass;
        }
        internal.mass = total_mass;

        m_nodes.push_back(internal);
    };

    for (USize i = children_start; i < children_end; ++i) {
        const MortonCode prefix = prefix_at_level(m_nodes[i].prefix, level);

        if (prefix != current_prefix) {
            flush_internal(i);
            current_prefix = prefix;
            group_start    = i;
        }
    }

    // NOTE: Flush final group.
    flush_internal(children_end);

    return {nodes_before, m_nodes.size()};
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::propagate_com_up() {
    // NOTE: COM is already computed during build_level, but we recompute
    // to ensure correctness after all nodes are created.
    // Process nodes in reverse order (children before parents).

    for (auto it = m_nodes.rbegin(); it != m_nodes.rend(); ++it) {
        Node& node = *it;

        if (node.is_leaf || node.is_empty()) {
            continue;
        }

        // NOTE: Recompute COM from children.
        Vec2  weighted_com{0.0, 0.0};
        Float total_mass = 0.0;

        for (USize i = 0; i < 4; ++i) {
            if (node.children[i] != NODE_INDEX_NULL) {
                const Node& child = m_nodes[node.children[i]];
                weighted_com.x += child.com.x * child.mass;
                weighted_com.y += child.com.y * child.mass;
                total_mass += child.mass;
            }
        }

        if (total_mass > 0.0) {
            node.com.x = weighted_com.x / total_mass;
            node.com.y = weighted_com.y / total_mass;
        }
        node.mass = total_mass;
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Vec2
BarnesHutMorton<Float, MortonCode>::compute_acceleration(
    Vec2 pos, USize skip_body_index) const {
    (void)skip_body_index;  // NOTE: We use position-based self-interaction
                            // detection instead.

    Vec2 acc = Vec2::make_zero();

    if (m_nodes.empty()) {
        return acc;
    }

    const Float softening_sq = m_softening * m_softening;

    // NOTE: Find root node - the node with the lowest level (closest to 0).
    // After tree construction, the root should be the last internal node added.
    NodeIndex root_idx   = NODE_INDEX_NULL;
    USize     root_level = MAX_DEPTH + 1;

    for (USize i = 0; i < m_nodes.size(); ++i) {
        const Node& node = m_nodes[i];
        if (node.level < root_level) {
            root_level = node.level;
            root_idx   = i;
        }
    }

    // NOTE: If we only have leaves (no internal nodes), iterate through all.
    if (root_idx == NODE_INDEX_NULL ||
        (m_nodes.size() > 1 && m_nodes[root_idx].is_leaf)) {
        for (USize i = 0; i < m_nodes.size(); ++i) {
            const Node& node = m_nodes[i];
            if (!node.is_leaf) {
                continue;
            }

            const Vec2  delta   = node.com.sub(pos);
            const Float dist_sq = delta.length_sq();

            // NOTE: Skip self-interaction based on distance (position match).
            if (dist_sq < static_cast<Float>(1e-20)) {
                continue;
            }

            const Float dist_norm =
                (dist_sq + softening_sq) * std::sqrt(dist_sq + softening_sq);
            const Float factor = m_g * node.mass / dist_norm;

            acc.x += factor * delta.x;
            acc.y += factor * delta.y;
        }
        return acc;
    }

    compute_acceleration_recursive(pos, root_idx, acc);
    return acc;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::compute_acceleration_recursive(
    Vec2 pos, NodeIndex node_idx, Vec2& acc) const {
    const Node& node = m_nodes[node_idx];

    if (node.is_empty()) {
        return;
    }

    const Vec2  delta   = node.com.sub(pos);
    const Float dist_sq = delta.length_sq();

    // NOTE: Skip self-interaction based on distance (position match).
    if (dist_sq < static_cast<Float>(1e-20)) {
        // NOTE: For internal nodes, recurse into children.
        if (!node.is_leaf) {
            for (USize i = 0; i < 4; ++i) {
                if (node.children[i] != NODE_INDEX_NULL) {
                    compute_acceleration_recursive(pos, node.children[i], acc);
                }
            }
        }
        return;
    }

    const Float dist = std::sqrt(dist_sq);

    // NOTE: Barnes-Hut opening criterion: s/d < theta.
    // If satisfied (or node is leaf), use this node's COM directly.
    const Float ratio = node.size / dist;

    if (node.is_leaf || ratio < m_theta) {
        // NOTE: Compute gravitational acceleration.
        const Float softening_sq = m_softening * m_softening;
        const Float dist_norm =
            (dist_sq + softening_sq) * std::sqrt(dist_sq + softening_sq);
        const Float factor = m_g * node.mass / dist_norm;

        acc.x += factor * delta.x;
        acc.y += factor * delta.y;
    } else {
        // NOTE: Node is too close, recurse into children.
        for (USize i = 0; i < 4; ++i) {
            if (node.children[i] != NODE_INDEX_NULL) {
                compute_acceleration_recursive(pos, node.children[i], acc);
            }
        }
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
U8 BarnesHutMorton<Float, MortonCode>::quadrant_at_level(MortonCode code,
                                                         USize      level) {
    // NOTE: Extract 2 bits at the given level.
    // Level 0 = top 2 bits, level 1 = next 2 bits, etc.
    // For 2D Morton codes, each level uses 2 bits (one for x, one for y
    // interleaved).
    const USize shift = MORTON_BITS - 2 * (level + 1);
    return static_cast<U8>((code >> shift) & 0x3);
}

template <FloatT Float, math::MortonCodeT MortonCode>
MortonCode BarnesHutMorton<Float, MortonCode>::prefix_at_level(MortonCode code,
                                                               USize level) {
    // NOTE: Mask out bits below the given level.
    // Level 0 = root (all nodes share the same prefix: 0)
    // Level 1 = top 2 bits, level 2 = top 4 bits, etc.
    if (level == 0) {
        return 0;  // Root level - all nodes share prefix 0
    }
    if (level >= MAX_DEPTH) {
        return code;
    }
    const USize      bits_to_keep = 2 * level;
    const USize      shift        = MORTON_BITS - bits_to_keep;
    const MortonCode mask         = (~MortonCode{0}) << shift;
    return code & mask;
}

template <FloatT Float, math::MortonCodeT MortonCode>
Float BarnesHutMorton<Float, MortonCode>::size_at_level(USize level) const {
    // NOTE: Root (level 0) has size = m_root_size.
    // Each level halves the size.
    return m_root_size / static_cast<Float>(USize{1} << level);
}

template class BarnesHutMorton<F32, U32>;
template class BarnesHutMorton<F32, U64>;
template class BarnesHutMorton<F64, U32>;
template class BarnesHutMorton<F64, U64>;

}  // namespace nbody::sim
