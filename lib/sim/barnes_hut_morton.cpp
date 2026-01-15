#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

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
      m_radix(config.radix),
      m_use_proper_verlet(config.use_proper_verlet),
      m_bounds_min(0),
      m_bounds_max(1),
      m_integrate_fn(config.integrate_fn) {
    m_morton_bodies.reserve(m_bodies.size());
    if (m_use_proper_verlet) {
        m_old_accelerations.resize(m_bodies.size(), Vec2::make_zero());
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::step(Float dt) {
    if (m_use_proper_verlet) {
        // NOTE: Proper Velocity Verlet algorithm.
        // (1) x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2.
        // (2) Compute a(t+dt) from new positions.
        // (3) v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt.

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

        build_tree();

        for (const MortonBody& mb : m_morton_bodies) {
            Vec2 acc                  = compute_acceleration(mb.body_idx);
            m_bodies[mb.body_idx].acc = m_bodies[mb.body_idx].acc.add(acc);
        }

        for (USize i = 0; i < m_bodies.size(); ++i) {
            Body& body = m_bodies[i];
            body.vel   = body.vel.add(
                m_old_accelerations[i].add(body.acc).scale(half * dt));
        }
    } else {
        if (m_parallel) {
            base::parallel_for_each(
                m_bodies.begin(), m_bodies.end(),
                [](Body& body) { body.acc = Vec2::make_zero(); });
        } else {
            for (Body& body : m_bodies) {
                body.acc = Vec2::make_zero();
            }
        }

        build_tree();

        if (m_parallel) {
            std::vector<Vec2> accelerations(m_bodies.size(), Vec2::make_zero());

            // NOTE: bodies have been reordered to match m_morton_bodies order,
            // so this access pattern is now linear in memory.
            base::parallel_for_each(
                m_morton_bodies.begin(), m_morton_bodies.end(),
                [this, &accelerations](const MortonBody& mb) {
                    accelerations[mb.body_idx] =
                        compute_acceleration(mb.body_idx);
                });

            for (USize i = 0; i < m_bodies.size(); ++i) {
                m_bodies[i].acc = m_bodies[i].acc.add(accelerations[i]);
            }
        } else {
            for (const MortonBody& mb : m_morton_bodies) {
                Vec2 acc                  = compute_acceleration(mb.body_idx);
                m_bodies[mb.body_idx].acc = m_bodies[mb.body_idx].acc.add(acc);
            }
        }

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
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::insert_body(Body&& body) {
    m_bodies.push_back(std::move(body));
    if (m_use_proper_verlet) {
        m_old_accelerations.push_back(Vec2::make_zero());
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
std::span<const typename BarnesHutMorton<Float, MortonCode>::Body,
          std::dynamic_extent>
BarnesHutMorton<Float, MortonCode>::bodies() const {
    return m_bodies;
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::build_tree() {
    if (m_bodies.empty()) {
        m_nodes.clear();
        m_morton_bodies.clear();
        return;
    }

    compute_morton_codes();
    sort_by_morton();

    // NOTE: Reorder bodies to match Morton order for better cache coherence.
    std::vector<Body> ordered_bodies;
    ordered_bodies.reserve(m_bodies.size());

    for (const MortonBody& mb : m_morton_bodies) {
        ordered_bodies.push_back(m_bodies[mb.body_idx]);
    }
    m_bodies = std::move(ordered_bodies);

    for (USize i = 0; i < m_morton_bodies.size(); ++i) {
        m_morton_bodies[i].body_idx = i;
    }

    m_nodes.clear();
    m_nodes.reserve(m_bodies.size() * 2);

    Vec2 root_center = Vec2{
        static_cast<Float>((m_bounds_min + m_bounds_max) / 2.0),
        static_cast<Float>((m_bounds_min + m_bounds_max) / 2.0),
    };
    Float root_size = static_cast<Float>(m_bounds_max - m_bounds_min);

    build_nodes_recursive(0, m_morton_bodies.size(), 0, 0, root_size,
                          root_center);
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::compute_morton_codes() {
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

    Float padding = std::max(static_cast<Float>(1e-6),
                             (x_max - x_min + y_max - y_min) * 0.01f);
    m_bounds_min  = std::min(x_min, y_min) - padding;
    m_bounds_max  = std::max(x_max, y_max) + padding;

    m_morton_bodies.clear();
    m_morton_bodies.reserve(m_bodies.size());

    for (USize i = 0; i < m_bodies.size(); ++i) {
        MortonCode code = math::encode_to_morton<Float, MortonCode>(
            m_bodies[i].pos.x, m_bodies[i].pos.y, m_bounds_min, m_bounds_max);
        m_morton_bodies.push_back(MortonBody{.morton = code, .body_idx = i});
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::sort_by_morton() {
    if (m_radix) {
        base::radix_sort(m_morton_bodies.begin(), m_morton_bodies.end(),
                         [](const MortonBody& a) { return a.morton; });
    } else {
        std::sort(m_morton_bodies.begin(), m_morton_bodies.end());
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::build_nodes_recursive(
    USize first, USize last, U8 level, MortonCode prefix, Float node_size,
    Vec2 node_center) {
    if (first >= last) {
        return;
    }

    USize node_idx = m_nodes.size();
    m_nodes.push_back(Node{
        .prefix         = prefix,
        .level          = level,
        .center_of_mass = Vec2::make_zero(),
        .total_mass     = 0,
        .size           = node_size,
        .first_idx      = first,
        .last_idx       = last,
        .children       = {Node::NODE_EMPTY, Node::NODE_EMPTY, Node::NODE_EMPTY,
                           Node::NODE_EMPTY},
    });

    if (last - first <= 1 || level >= MAX_DEPTH) {
        compute_node_properties(node_idx);
        return;
    }

    const USize          bits_to_shift   = MORTON_BITS - 2 * (level + 1);
    std::array<USize, 5> quadrant_bounds = {first, first, first, first, last};

    USize current_pos = first;
    for (U8 q = 0; q < 4; ++q) {
        MortonCode quadrant_prefix =
            prefix | (static_cast<MortonCode>(q) << bits_to_shift);
        MortonCode quadrant_max =
            quadrant_prefix |
            ((static_cast<MortonCode>(1) << bits_to_shift) - 1);

        auto it = std::upper_bound(
            m_morton_bodies.begin() + static_cast<std::ptrdiff_t>(current_pos),
            m_morton_bodies.begin() + static_cast<std::ptrdiff_t>(last),
            quadrant_max, [](MortonCode val, const MortonBody& mb) {
                return val < mb.morton;
            });

        quadrant_bounds[q + 1] =
            static_cast<USize>(it - m_morton_bodies.begin());

        current_pos = quadrant_bounds[q + 1];
    }

    Float child_size     = node_size / 2;
    Float quarter_offset = node_size / 4;

    std::array<Vec2, 4> child_centers = {
        Vec2{node_center.x - quarter_offset,
             node_center.y - quarter_offset}, // SW (00)
        Vec2{node_center.x + quarter_offset,
             node_center.y - quarter_offset}, // SE (01)
        Vec2{node_center.x - quarter_offset,
             node_center.y + quarter_offset}, // NW (10)
        Vec2{node_center.x + quarter_offset,
             node_center.y + quarter_offset}, // NE (11)
    };

    for (U8 q = 0; q < 4; ++q) {
        USize q_first = quadrant_bounds[q];
        USize q_last  = quadrant_bounds[q + 1];

        if (q_first < q_last) {
            MortonCode child_prefix =
                prefix | (static_cast<MortonCode>(q) << bits_to_shift);
            m_nodes[node_idx].children[q] = m_nodes.size();
            build_nodes_recursive(q_first, q_last, level + 1, child_prefix,
                                  child_size, child_centers[q]);
        }
    }

    compute_node_properties(node_idx);
}

template <FloatT Float, math::MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::compute_node_properties(
    USize node_idx) {
    Node& node = m_nodes[node_idx];

    if (node.is_leaf()) {
        Float total_mass   = 0;
        Vec2  weighted_pos = Vec2::make_zero();

        for (USize i = node.first_idx; i < node.last_idx; ++i) {
            const Body& body = m_bodies[m_morton_bodies[i].body_idx];
            total_mass += body.mass;
            weighted_pos = weighted_pos.add(body.pos.scale(body.mass));
        }

        node.total_mass = total_mass;
        if (total_mass > 0) {
            node.center_of_mass = weighted_pos.scale(1 / total_mass);
        } else {
            node.center_of_mass = Vec2::make_zero();
        }
    } else {
        Float total_mass   = 0;
        Vec2  weighted_pos = Vec2::make_zero();

        for (USize i = 0; i < 4; ++i) {
            if (node.children[i] != Node::NODE_EMPTY) {
                const Node& child = m_nodes[node.children[i]];
                total_mass += child.total_mass;
                weighted_pos = weighted_pos.add(
                    child.center_of_mass.scale(child.total_mass));
            }
        }

        node.total_mass = total_mass;
        if (total_mass > 0) {
            node.center_of_mass = weighted_pos.scale(1 / total_mass);
        } else {
            node.center_of_mass = Vec2::make_zero();
        }
    }
}

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Vec2
BarnesHutMorton<Float, MortonCode>::compute_acceleration(USize body_idx) const {
    if (m_nodes.empty()) {
        return Vec2::make_zero();
    }

    return compute_acceleration_from_node(0, m_bodies[body_idx].pos);
}

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Vec2
BarnesHutMorton<Float, MortonCode>::compute_acceleration_from_node(
    USize root_node_idx, Vec2 pos) const {
    // WHY: Use an explicit stack to avoid recursion overhead.
    std::array<USize, 256> stack{0};
    USize                  stack_top      = 0;
    constexpr USize        STACK_CAPACITY = 256;

    stack[stack_top++] = root_node_idx;

    Vec2        acc          = Vec2::make_zero();
    const Float softening_sq = m_softening * m_softening;
    const Float theta_sq     = m_theta * m_theta;

    while (stack_top > 0) {
        USize       current_idx  = stack[--stack_top];
        const Node& current_node = m_nodes[current_idx];

        if (current_node.total_mass == 0) {
            continue;
        }

        Vec2        delta   = current_node.center_of_mass.sub(pos);
        const Float dist_sq = delta.length_sq();

        if (current_node.is_leaf() && current_node.body_count() == 1) {
            if (dist_sq < softening_sq * 0.01f) {
                continue;
            }
        }

        const Float size_sq       = current_node.size * current_node.size;
        const bool  is_far_enough = size_sq < theta_sq * dist_sq;

        if (current_node.is_leaf() || is_far_enough) {
            const Float dist_norm =
                (dist_sq + softening_sq) * std::sqrt(dist_sq + softening_sq);
            const Float factor = m_g * current_node.total_mass / dist_norm;

            acc.x += factor * delta.x;
            acc.y += factor * delta.y;
        } else {
            if (stack_top + 4 >= STACK_CAPACITY) {
                continue;
            }

            for (USize i = 0; i < 4; ++i) {
                if (current_node.children[i] != Node::NODE_EMPTY) {
                    stack[stack_top++] = current_node.children[i];
                }
            }
        }
    }

    return acc;
}

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Vec2
BarnesHutMorton<Float, MortonCode>::get_node_center_from_prefix(
    MortonCode prefix, U8 level) const {
    Float size   = static_cast<Float>(m_bounds_max - m_bounds_min);
    Vec2  center = Vec2{
        static_cast<Float>((m_bounds_min + m_bounds_max) / 2.0),
        static_cast<Float>((m_bounds_min + m_bounds_max) / 2.0),
    };

    for (U8 l = 0; l < level; ++l) {
        size /= 2;
        Float offset = size / 2;

        U8 quadrant = get_quadrant(prefix, l);

        if (quadrant & 1) {
            center.x += offset;
        } else {
            center.x -= offset;
        }

        if (quadrant & 2) {
            center.y += offset;
        } else {
            center.y -= offset;
        }
    }

    return center;
}

template <FloatT Float, math::MortonCodeT MortonCode>
Float BarnesHutMorton<Float, MortonCode>::get_node_size(U8 level) const {
    Float size = static_cast<Float>(m_bounds_max - m_bounds_min);
    for (U8 l = 0; l < level; ++l) {
        size /= 2;
    }
    return size;
}

template <FloatT Float, math::MortonCodeT MortonCode>
U8 BarnesHutMorton<Float, MortonCode>::get_quadrant(MortonCode code,
                                                    U8         level) const {
    USize shift = MORTON_BITS - 2 * (level + 1);
    return static_cast<U8>((code >> shift) & 0x3);
}

template class BarnesHutMorton<F32, U32>;
template class BarnesHutMorton<F32, U64>;
template class BarnesHutMorton<F64, U32>;
template class BarnesHutMorton<F64, U64>;

}  // namespace nbody::sim
