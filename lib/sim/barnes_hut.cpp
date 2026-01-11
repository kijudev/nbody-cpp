#include "sim/barnes_hut.hpp"

#include <algorithm>

#include "base/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
BarnesHut<Float>::BarnesHut(const Config& config)
    : m_bodies(std::move(config.bodies)),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta),
      m_integrate_fn(config.integrate_fn) {}

template <FloatT Float>
void BarnesHut<Float>::step(Float dt) {
    for (Body& body : m_bodies) {
        body.acc = Vec2::make_zero();
    }

    Quad root_quad = Quad::make_containing_bodies(m_bodies);
    m_quad_tree.clear(root_quad);

    for (const Body& body : m_bodies) {
        m_quad_tree.insert(body.pos, body.mass);
    }

    m_quad_tree.propagate_up_pos_mass();

    for (Body& body : m_bodies) {
        Vec2 acc = m_quad_tree.propagate_down_acc(body.pos, m_g, m_softening, m_theta);
        body.acc = body.acc.add(acc);
    }

    for (Body& body : m_bodies) {
        m_integrate_fn(body, dt);
    }
}

template <FloatT Float>
std::span<const typename BarnesHut<Float>::Body, std::dynamic_extent> BarnesHut<Float>::bodies()
    const {
    return m_bodies;
}

template <FloatT Float>
typename BarnesHut<Float>::Quad BarnesHut<Float>::Quad::make_containing_bodies(
    std::span<Body, std::dynamic_extent> bodies) {
    Float x_min = std::numeric_limits<Float>::max();
    Float x_max = std::numeric_limits<Float>::lowest();
    Float y_min = std::numeric_limits<Float>::max();
    Float y_max = std::numeric_limits<Float>::lowest();

    for (const Body& body : bodies) {
        x_min = std::min(x_min, body.pos.x);
        x_max = std::max(x_max, body.pos.x);
        y_min = std::min(y_min, body.pos.y);
        y_max = std::max(y_max, body.pos.y);
    }

    Vec2 center = Vec2{
        static_cast<Float>(x_min + (x_max - x_min) / 2.0),
        static_cast<Float>(y_min + (y_max - y_min) / 2.0),
    };
    Float size = std::max(x_max - x_min, y_max - y_min);

    return Quad{
        .center = center,
        .size   = size,
    };
}

template <FloatT Float>
typename BarnesHut<Float>::Quad BarnesHut<Float>::Quad::into_quad(QuadIndex quad_index) const {
    const Float half = size / 2.0;

    switch (quad_index) {
        case 0:  // NE
            return Quad{
                .center = Vec2(center.x + half / 2.0, center.y + half / 2.0),
                .size   = half,
            };
        case 1:  // NW
            return Quad{
                .center = Vec2(center.x - half / 2.0, center.y + half / 2.0),
                .size   = half,
            };
        case 2:  // SW
            return Quad{
                .center = Vec2(center.x - half / 2.0, center.y - half / 2.0),
                .size   = half,
            };
        case 3:  // SE
            return Quad{
                .center = Vec2(center.x + half / 2.0, center.y - half / 2.0),
                .size   = half,
            };
        default:
            return Quad{
                .center = Vec2::make_zero(),
                .size   = 0.0,
            };
    }
}

template <FloatT Float>
std::array<typename BarnesHut<Float>::Quad, 4> BarnesHut<Float>::Quad::into_quads() const {
    Float half = size / 2.0;

    return std::array<Quad, 4>{
        Quad{
             .center = Vec2(center.x + half / 2.0, center.y + half / 2.0),
             .size   = half,
             },
        Quad{
             .center = Vec2(center.x - half / 2.0,                                    center.y + half / 2.0),
             .size   = half,
             },
        Quad{
             .center = Vec2(center.x - half / 2.0,center.y - half / 2.0),
             .size   = half,
             },
        Quad{
             .center = Vec2(center.x + half / 2.0,                                                  center.y - half / 2.0),
             .size   = half,
             },
    };
}

template <FloatT Float>
typename BarnesHut<Float>::Quad BarnesHut<Float>::Quad::quad_from_pos(Vec2 pos) const {
    const bool is_north = pos.y >= center.y;
    const bool is_east  = pos.x >= center.x;

    if (is_north && is_east) {
        return into_quad(0);
    } else if (is_north && !is_east) {
        return into_quad(1);
    } else if (!is_north && !is_east) {
        return into_quad(2);
    } else {
        return into_quad(3);
    }
}

template <FloatT Float>
BarnesHut<Float>::QuadIndex BarnesHut<Float>::Quad::quad_index_from_pos(Vec2 pos) const {
    QuadIndex index = 0;

    if (pos.x < center.x) {
        index |= 1;
    }

    if (pos.y < center.y) {
        index |= 2;
    }

    return index;
}

template <FloatT Float>
BarnesHut<Float>::Node BarnesHut<Float>::Node::make_empty(NodeIndex next_index, Quad quad) {
    return Node{
        .children_index = NODE_INDEX_EMPTY,
        .next_index     = next_index,
        .quad           = quad,
        .pos            = Vec2::make_zero(),
        .mass           = 0.0,
    };
}

template <FloatT Float>
bool BarnesHut<Float>::Node::is_empty() const {
    return mass == 0.0;
}

template <FloatT Float>
bool BarnesHut<Float>::Node::is_internal() const {
    return children_index != NODE_INDEX_EMPTY;
}

template <FloatT Float>
bool BarnesHut<Float>::Node::is_leaf() const {
    return !is_empty() && !is_internal();
}

template <FloatT Float>
void BarnesHut<Float>::QuadTree::clear(Quad quad) {
    nodes.clear();
    parent_indices.clear();
    nodes.emplace_back(Node::make_empty(NODE_INDEX_EMPTY, quad));
}

template <FloatT Float>
typename BarnesHut<Float>::NodeIndex BarnesHut<Float>::QuadTree::subdivide(NodeIndex node_index) {
    parent_indices.push_back(node_index);

    const NodeIndex children_index   = nodes.size();
    nodes[node_index].children_index = children_index;

    const std::array<NodeIndex, 4> next_indices = {
        children_index + 1,
        children_index + 2,
        children_index + 3,
        nodes[node_index].next_index,
    };

    const std::array<Quad, 4> quads = nodes[node_index].quad.into_quads();

    for (USize i = 0; i < 4; ++i) {
        nodes.push_back(Node::make_empty(next_indices[i], quads[i]));
    }

    return children_index;
}

template <FloatT Float>
void BarnesHut<Float>::QuadTree::insert(Vec2 pos, Float mass) {
    NodeIndex node_index = NODE_INDEX_EMPTY;

    while (nodes[node_index].is_internal()) {
        QuadIndex q = nodes[node_index].quad.quad_index_from_pos(pos);
        node_index  = nodes[node_index].children_index + q;
    }

    if (nodes[node_index].is_empty()) {
        nodes[node_index].pos  = pos;
        nodes[node_index].mass = mass;
        return;
    }

    const Vec2  ex_pos  = nodes[node_index].pos;
    const Float ex_mass = nodes[node_index].mass;

    if (pos.is_approx_equal(ex_pos)) {
        nodes[node_index].mass += mass;
        return;
    }

    while (true) {
        const NodeIndex children_index = subdivide(node_index);

        const QuadIndex q1    = nodes[node_index].quad.quad_index_from_pos(ex_pos);
        const QuadIndex q2    = nodes[node_index].quad.quad_index_from_pos(pos);
        const Quad      quad1 = nodes[node_index].quad.quad_from_pos(ex_pos);
        const Quad      quad2 = nodes[node_index].quad.quad_from_pos(pos);

        if (quad1.center.is_approx_equal(quad2.center)) {
            node_index = children_index + q1;
        } else {
            const NodeIndex n1 = children_index + q1;
            const NodeIndex n2 = children_index + q2;

            nodes[n1].pos  = ex_pos;
            nodes[n1].mass = ex_mass;
            nodes[n2].pos  = pos;
            nodes[n2].mass = mass;

            return;
        }
    }
}

template <FloatT Float>
void BarnesHut<Float>::QuadTree::propagate_up_pos_mass() {
    for (NodeIndex parent_index : parent_indices) {
        const NodeIndex i = nodes[parent_index].children_index;

        nodes[parent_index].pos = nodes[i]
                                      .pos.scale(nodes[i].mass)
                                      .add(nodes[i + 1].pos.scale(nodes[i + 1].mass))
                                      .add(nodes[i + 2].pos.scale(nodes[i + 2].mass))
                                      .add(nodes[i + 3].pos.scale(nodes[i + 3].mass));

        nodes[parent_index].mass =
            nodes[i].mass + nodes[i + 1].mass + nodes[i + 2].mass + nodes[i + 3].mass;

        nodes[parent_index].pos = nodes[parent_index].pos.scale(1.0 / nodes[parent_index].mass);
    }
}

template <FloatT Float>
BarnesHut<Float>::Vec2 BarnesHut<Float>::QuadTree::propagate_down_acc(Vec2 pos, Float g,
                                                                      Float softening,
                                                                      Float theta) const {
    Vec2 acc = Vec2::make_zero();

    const Float theta_sq     = theta * theta;
    const Float softening_sq = softening * softening;

    // NOTE: Start from the root node.
    NodeIndex node_index = 0;

    while (true) {
        const Node& node            = nodes[node_index];
        const Float dist            = node.pos.sub(pos).length();
        const Float dist_sq         = node.pos.sub(pos).length_sq();
        const bool  is_sd_satisfied = (node.quad.size * node.quad.size) < (theta_sq * dist_sq);

        if (node.is_leaf() || is_sd_satisfied) {
            const Float dist_norm = (dist_sq + softening_sq) * std::sqrt(dist_sq);

            acc.x += std::min(g * node.mass * dist / dist_norm, std::numeric_limits<Float>::max());

            acc.y += std::min(g * node.mass * dist / dist_norm, std::numeric_limits<Float>::max());

            if (node.next_index == NODE_INDEX_EMPTY) {
                break;
            }

            node_index = node.next_index;
        } else {
            node_index = node.children_index;
        }
    }

    return acc;
}

template class BarnesHut<F32>;
template class BarnesHut<F64>;
}  // namespace nbody::sim
