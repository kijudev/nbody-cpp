#pragma once

#include <memory>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "type.hpp"

namespace nbody {
template <FloatT Float>
class Sim2LinearBarnesHut {
   public:
    using Vec2        = Vec2T<Float>;
    using Body        = Body2T<Float>;
    using Layout      = std::vector<Body>;
    using IntegrateFn = Integrate2Fn<Float>;

    struct Node {
        Vec2  center{0.0, 0.0};
        Float mass{0.0};
        Float radius{0.0};
        Vec2  com{0.0, 0.0};

        Vec2  body_pos{0.0, 0.0};
        Float body_mass{0.0};
        bool  has_body{false};

        std::array<std::unique_ptr<Node>, 4> children{};

        static constexpr USize QUAD_NE = 0;
        static constexpr USize QUAD_NW = 1;
        static constexpr USize QUAD_SW = 2;
        static constexpr USize QUAD_SE = 3;

        Node() = default;

        USize get_quadrant_of_point(const Vec2& point) const {
            if (point.x >= center.x && point.y >= center.y) return QUAD_NE;
            if (point.x < center.x && point.y >= center.y) return QUAD_NW;
            if (point.x < center.x && point.y < center.y) return QUAD_SW;
            return QUAD_SE;
        }

        void ensure_child(USize q) {
            if (children[q]) return;

            Float hr = radius * 0.5;  // child half-radius
            Vec2  c  = center;
            switch (q) {
                case QUAD_NE:
                    c.x += hr;
                    c.y += hr;
                    break;
                case QUAD_NW:
                    c.x -= hr;
                    c.y += hr;
                    break;
                case QUAD_SW:
                    c.x -= hr;
                    c.y -= hr;
                    break;
                case QUAD_SE:
                    c.x += hr;
                    c.y -= hr;
                    break;
            }

            auto child      = std::make_unique<Node>();
            child->center   = c;
            child->radius   = hr;
            child->mass     = 0.0;
            child->com      = Vec2::zero();
            child->has_body = false;
            children[q]     = std::move(child);
        }

        void insert_body(const Vec2& p, Float m) {
            // 1) Update this node's aggregated mass and center-of-mass
            if (mass == 0.0) {
                com  = p;
                mass = m;
            } else {
                // new_com = (com * mass + p * m) / (mass + m)
                Vec2 weighted = com.scale(mass).add(p.scale(m));
                mass          = mass + m;
                com           = weighted.scale(1.0 / mass);
            }

            // 2) If this node is a leaf (no children)
            bool is_leaf = !(children[0] || children[1] || children[2] || children[3]);
            if (is_leaf) {
                if (!has_body) {
                    // empty leaf: store the body here
                    body_pos  = p;
                    body_mass = m;
                    has_body  = true;
                    return;
                } else {
                    // leaf already contains a body. If the new body is at the exact same
                    // position as the stored one, accumulate mass and keep it as a single stored
                    // body
                    Vec2  delta = p.sub(body_pos);
                    Float dist2 = delta.length_sq();
                    if (dist2 == static_cast<Float>(0.0) || radius == static_cast<Float>(0.0)) {
                        // Can't subdivide further (or bodies coincide) — combine masses at this
                        // node. Update the stored body as the combined center (body_pos becomes
                        // com).
                        body_pos  = com;
                        body_mass = mass;
                        has_body  = true;
                        return;
                    }

                    // Otherwise, subdivide: move the existing stored body into the appropriate
                    // child.
                    USize existing_q = get_quadrant_of_point(body_pos);
                    ensure_child(existing_q);
                    children[existing_q]->insert_body(body_pos, body_mass);
                    has_body = false;  // this node no longer stores a direct body
                    // fallthrough to insert the incoming body into a child below
                }
            }

            // 3) Insert the body into the correct child (this node is internal, or was just
            // subdivided)
            USize q = get_quadrant_of_point(p);
            ensure_child(q);
            children[q]->insert_body(p, m);
        }
    };

    void step(Float dt) {
        for (Body& body : m_bodies) {
            body.acc = Vec2::zero();
        }

        // Build the Barnes-Hut tree for this timestep
        tree_construct();

        // Compute accelerations using the constructed tree
        apply_gravity();

        // Integrate bodies
        for (Body& body : m_bodies) {
            m_integrate(body, dt);
        }

        // Free the tree to avoid holding stale structure (will be rebuilt next step)
        m_tree_root.reset();
    }

    Sim2LinearBarnesHut() = default;
    Sim2LinearBarnesHut(std::vector<Body> bodies, IntegrateFn integrator)
        : m_bodies(std::move(bodies)), m_integrate(std::move(integrator)) {}

    [[nodiscard]] const std::vector<Body>& bodies() const noexcept { return m_bodies; }
    [[nodiscard]] std::vector<Body>&       bodies_mut() noexcept { return m_bodies; }

   public:
    Layout                m_bodies{};
    std::unique_ptr<Node> m_tree_root{};

    const IntegrateFn m_integrate{};
    const Float       m_g{};
    const Float       m_softening{};

    void apply_gravity() {
        if (!m_tree_root) return;

        // Barnes-Hut opening angle (tunable)
        constexpr Float theta = static_cast<Float>(0.5);

        // Recursive visitor that accumulates acceleration contribution from `node` onto `body`.
        std::function<void(Body&, Node*)> visit = [&](Body& body, Node* node) {
            if (!node || node->mass == static_cast<Float>(0.0)) return;

            // Vector from body to node's center-of-mass
            Vec2 delta = node->com.sub(body.pos);
            // softened squared distance
            Float dist2 = delta.length_sq() + (m_softening * m_softening);
            Float dist  = std::sqrt(dist2);

            bool is_leaf =
                !(node->children[0] || node->children[1] || node->children[2] || node->children[3]);

            if (is_leaf) {
                // If leaf stores a body, make sure we don't self-interact
                if (node->has_body) {
                    // Heuristic check for same body: identical position and mass
                    if (node->body_pos.x == body.pos.x && node->body_pos.y == body.pos.y &&
                        node->body_mass == body.mass) {
                        return;
                    }

                    // Treat the leaf as a single body located at node->com
                    Float inv_r3  = static_cast<Float>(1.0) / (dist * dist2);
                    Vec2  contrib = delta.scale(m_g * node->mass * inv_r3);
                    body.acc      = body.acc.add(contrib);
                }
                return;
            }

            // Opening criterion: s / d < theta, where s is node size (use 2 * radius for full
            // width)
            Float s = node->radius * static_cast<F>(2.0);
            if (dist > static_cast<F>(0.0) && (s / dist) < theta) {
                // Accept this node as a single body at its center-of-mass
                Float inv_r3  = static_cast<F>(1.0) / (dist * dist2);
                Vec2  contrib = delta.scale(G * node->mass * inv_r3);
                body.acc      = body.acc.add(contrib);
            } else {
                // Otherwise, recurse into children
                for (const auto& child : node->children) {
                    if (child) visit(body, child.get());
                }
            }
        };

        // Accumulate acceleration for each body using the tree
        for (Body& body : m_bodies) {
            visit(body, m_tree_root.get());
        }
    }

    // TODO: Refactor into impl.
    //
    // Public visualization helpers:
    // - `Quad` is a small POD describing a node's region for rendering.
    // - `collect_quads` fills a vector with every node in the tree (for external rendering code).
    // - `draw_quads` accepts a callable that will be invoked for every node (center, radius,
    // is_leaf).
    struct Quad {
        Vec2  center;
        Float radius;
        bool  is_leaf;
        Float mass;
        Vec2  com;
    };

    void collect_quads(std::vector<Quad>& out) const {
        if (!m_tree_root) return;

        std::function<void(const Node*)> visit = [&](const Node* node) {
            if (!node) return;
            bool is_leaf =
                !(node->children[0] || node->children[1] || node->children[2] || node->children[3]);
            out.push_back(Quad{node->center, node->radius, is_leaf, node->mass, node->com});
            for (const auto& c : node->children) {
                if (c) visit(c.get());
            }
        };

        visit(m_tree_root.get());
    }

    template <typename DrawFn>
    void draw_quads(const DrawFn& draw_fn) const {
        if (!m_tree_root) return;

        std::function<void(const Node*)> visit = [&](const Node* node) {
            if (!node) return;
            bool is_leaf =
                !(node->children[0] || node->children[1] || node->children[2] || node->children[3]);
            // draw_fn(center, radius, is_leaf, mass, com)
            draw_fn(node->center, node->radius, is_leaf, node->mass, node->com);
            for (const auto& c : node->children) {
                if (c) visit(c.get());
            }
        };

        visit(m_tree_root.get());
    }

    void apply_gravity_pair(Body& a, Body& b) {
        Vec2  delta     = b.pos.sub(a.pos);
        Float r2_soft   = delta.length_sq() + (m_softening * m_softening);
        Float inv_r3    = 1.0 / (std::sqrt(r2_soft) * r2_soft);
        Vec2  a_contrib = delta.scale(m_g * b.mass * inv_r3);
        Vec2  b_contrib = delta.scale(m_g * a.mass * inv_r3);

        a.acc = a.acc.add(a_contrib);
        b.acc = b.acc.sub(b_contrib);
    }

    void tree_construct() {
        if (!m_tree_root) {
            m_tree_root         = std::make_unique<Node>();
            m_tree_root->center = Vec2::zero();
            m_tree_root->mass   = 0.0;
            m_tree_root->radius = 0.0;
        }

        Float extent_x_min = std::numeric_limits<Float>::max();
        Float extent_x_max = std::numeric_limits<Float>::lowest();
        Float extent_y_min = std::numeric_limits<Float>::max();
        Float extent_y_max = std::numeric_limits<Float>::lowest();

        for (const Body& body : m_bodies) {
            extent_x_min = std::min(extent_x_min, body.pos.x);
            extent_x_max = std::max(extent_x_max, body.pos.x);
            extent_y_min = std::min(extent_y_min, body.pos.y);
            extent_y_max = std::max(extent_y_max, body.pos.y);
        }

        Vec2 center = Vec2((extent_x_min + extent_x_max) / static_cast<Float>(2.0),
                           (extent_y_min + extent_y_max) / static_cast<Float>(2.0));
        // radius is the half-extent that covers all bodies
        Float radius = std::max(extent_x_max - center.x, center.x - extent_x_min);
        radius       = std::max(radius, extent_y_max - center.y);
        radius       = std::max(radius, center.y - extent_y_min);

        m_tree_root->center = center;
        m_tree_root->radius = radius;

        // Clear previous tree children and data
        m_tree_root->mass      = 0.0;
        m_tree_root->com       = Vec2::zero();
        m_tree_root->has_body  = false;
        m_tree_root->body_mass = 0.0;
        m_tree_root->children  = {};

        for (const Body& body : m_bodies) {
            m_tree_root->insert_body(body.pos, body.mass);
        }
    }
};
}  // namespace nbody
