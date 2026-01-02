#pragma once

#include <cmath>
#include <memory>
#include <span>

#include "base/assert.hpp"
#include "base/type.hpp"
#include "nbody2/const.hpp"
#include "nbody2/euler.hpp"
#include "type.hpp"

namespace nbody2 {
template <FloatT Float>
class SimBarnesHut {
   public:
    // NOTE: Type aliases; for convenience.
    using Vec2        = math::Vec2T<Float>;
    using Body        = BodyT<Float>;
    using PointMass   = PointMassT<Float>;
    using Layout      = std::vector<Body>;
    using IntegrateFn = IntegrateFn<Float>;

    // NOTE: Quad info. Public; to be used by the renderer.
    using Quad                                    = USize;
    static constexpr Quad  QUAD_NE                = 0;
    static constexpr Quad  QUAD_NW                = 1;
    static constexpr Quad  QUAD_SE                = 2;
    static constexpr Quad  QUAD_SW                = 3;
    static constexpr Float M_MIN_ROOT_QUAD_RADIUS = 1.0;

    // NOTE: Forward declarations. Public; to be used by the renderer.
    enum class NodeKind;
    struct Node;

    struct Config {
        Layout      bodies{};
        IntegrateFn integrate_fn{euler_integrate_body<Float>};
        Float       g{G_IRL};
        Float       softening{SOFTENING_IRL};
    };

    SimBarnesHut(const Config& config)
        : m_bodies(std::move(config.bodies)),
          m_integrate(std::move(config.integrate_fn)),
          m_g(config.g),
          m_softening(config.softening) {}

    // NOTE: Returns an immutable reference to the bodies store.
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const { return m_bodies; }

    // NOTE: Adds a body to the simulation.
    void add_body(const Body& body) { m_bodies.push_back(body); }

    // NOTE: Performs a single step of the simulation.
    // 1) Initialize all bodies' accelerations to zero.
    // 2) Construct the Barnes-Hut quadtree.
    // 3) Apply gravity to each body.
    // 4) Integrate the bodies' positions and velocities.
    void step(Float dt) {
        for (Body& body : m_bodies) {
            body.acc = Vec2::zero();
        }

        impl_construct_tree();
        impl_apply_gravity();

        for (Body& body : m_bodies) {
            m_integrate(body, dt);
        }
    }

    std::vector<Node> collect_tree_nodes() {
        if (!m_root) {
            return {};
        }

        std::vector<Node> nodes{m_root};

        while (!nodes.empty()) {
            Node& node = nodes.back();
            nodes.pop_back();

            if (node.is_external()) {
                continue;
            }

            nodes.push_back(node.children[0]);
            nodes.push_back(node.children[1]);
            nodes.push_back(node.children[2]);
            nodes.push_back(node.children[3]);
        }

        return nodes;
    }

   private:
    Layout                m_bodies{};       // NOTE: Stores the bodies in the simulation.
    std::unique_ptr<Node> m_root{nullptr};  // NOTE: Stores the Barnes-Hut quadtree.

    const IntegrateFn m_integrate{};  // NOTE: Integrates the bodies' positions and velocities.
    const Float       m_g{};          // NOTE: Gravitational constant. can be changed at runtime.
    Float             m_softening{};  // NOTE: Softening parameter; can be changed at runtime.

    // NOTE: Applies gravity to each body.
    void impl_apply_gravity() {
        for (Body& target_body : m_bodies) {
            m_root->apply_gravity_target(target_body, m_g, m_softening);
        }
    }

    // NOTE: Constructs the Barnes-Hut quadtree.
    void impl_construct_tree() {
        impl_init_root();

        for (const Body& body : m_bodies) {
            m_root->insert_point_mass(body.pm);
        }
    }

    // NOTE: Initializes the root node of the Barnes-Hut quadtree.
    void impl_init_root() {
        if (m_bodies.empty()) {
            m_root = Node::make_internal(Vec2::zero(), 0.0);
            return;
        }

        auto [center, radius] = impl_root_node_center_radius();

        m_root = Node::make_internal(center, radius);
    }

    // NOTE: Calculates the center and radius of the root node of the Barnes-Hut quadtree. Ensures
    // that the radius is at least M_MIN_ROOT_QUAD_RADIUS.
    std::tuple<Vec2, Float> impl_root_node_center_radius() {
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

        Vec2 center =
            Vec2{static_cast<Float>(min_x + width / 2.0), static_cast<Float>(min_y + height / 2.0)};

        return std::make_tuple(center, radius);
    }

   public:
    // NOTE:
    // - INTERNAL: Represents a node that subdivides the space into four quadrants.
    // - EXTERNAL: Represents a node that contains a single body.
    enum class NodeKind { INTERNAL, EXTERNAL };

    struct Node {
       public:
        Vec2  center{0.0, 0.0};
        Float mass{0.0};
        Float radius{0.0};

        // TODO: This might be not needed; a temporary solution.
        NodeKind kind{NodeKind::INTERNAL};

        // WHY: It is necessary to store the position of the body in the node for external nodes.
        Vec2 body_pos{0.0, 0.0};

        std::array<std::unique_ptr<Node>, 4> children{nullptr, nullptr, nullptr, nullptr};

        static std::unique_ptr<Node> make_internal(const Vec2& center, Float radius) {
            return std::make_unique<Node>(Node{.center   = center,
                                               .mass     = 0.0,
                                               .radius   = radius,
                                               .kind     = NodeKind::INTERNAL,
                                               .body_pos = center});
        }

        static std::unique_ptr<Node> make_external(const Vec2& center, Float radius,
                                                   const PointMass& pm) {
            return std::make_unique<Node>(Node{.center   = center,
                                               .mass     = pm.mass,
                                               .radius   = radius,
                                               .kind     = NodeKind::EXTERNAL,
                                               .body_pos = pm.pos});
        }

        // NOTE: Inserting a point mass into the quad tree.
        // 1) Calculate the quadrant of the point mass.
        // 2) If the node is internal and the quadrant is empty, create a new external node.
        // 3) If the node is internal and the quadrant is not empty, insert the point mass into the
        // child node.
        // 4) If the node is external, create a new internal node and insert the point
        // mass into the appropriate child node.
        void insert_point_mass(const PointMass& pm) {
            Quad pm_quad = impl_pos_quad(pm.pos);

            if (is_internal()) {
                if (!children[pm_quad]) {
                    children[pm_quad] = make_external(impl_quad_center(pm_quad), radius / 2, pm);
                } else {
                    children[pm_quad]->insert_point_mass(pm);
                }
            } else {
                Quad sub_quad      = impl_pos_quad(body_pos);
                children[sub_quad] = make_external(impl_quad_center(sub_quad), radius / 2, pm);

                kind = NodeKind::INTERNAL;
                insert_point_mass(pm);
            }

            self_update_mass(pm);
            self_update_com(pm);
        }

        // NOTE: Apply gravity from this node to the target body.
        // 1) If the node is external, apply gravity from the point mass to the target body;
        // directly calculate the force.
        // 2) If the node is internal, calculate the distance between
        // the center of the node and the target body.
        // 3) If the distance is less than the
        // threshold, apply gravity from the point mass to the target body.
        // 4) If the distance is greater than the threshold, apply gravity from the center of the
        // node to the target body.
        void apply_gravity_target(Body& target, Float g, Float softening) {
            if (is_external()) {
                impl_apply_gravity_target_source(target, PointMass{.pos = center, .mass = mass}, g,
                                                 softening);
            } else {
                Float constexpr theta = 0.5;
                Float sd              = radius / target.pm.pos.sub(center).length();

                if (sd < theta) {
                    impl_apply_gravity_target_source(target, PointMass{.pos = center, .mass = mass},
                                                     g, softening);
                } else {
                    for (std::unique_ptr<Node>& child : children) {
                        if (child) {
                            child->apply_gravity_target(target, g, softening);
                        }
                    }
                }
            }
        }

        bool is_internal() const { return kind == NodeKind::INTERNAL; }
        bool is_external() const { return kind == NodeKind::EXTERNAL; }

        std::string fmt() const {
            return std::format(
                "Node{ center: {}, mass: {}, radius: {}, body_pos: {}, children: {} }",
                center.fmt(), std::to_string(mass), std::to_string(radius), body_pos.fmt(),
                std::to_string(children.size()));
        }

       private:
        void self_update_mass(const PointMass& pm) {
            ASSERT(is_internal(), "Node is not internal");
            mass += pm.mass;
        }

        void self_update_com(const PointMass& pm) {
            ASSERT(is_internal(), "Node is not internal");
            center = center.scale(mass).add(pm.pos.scale(pm.mass));
            center = center.scale(1 / (mass + pm.mass));
        }

        Quad impl_pos_quad(const Vec2& pos) const {
            if (pos.x >= center.x) {
                if (pos.y >= center.y) {
                    return QUAD_NE;
                } else {
                    return QUAD_SE;
                }
            } else {
                if (pos.y >= center.y) {
                    return QUAD_NW;
                } else {
                    return QUAD_SW;
                }
            }
        }

        Vec2 impl_quad_center(Quad quad) const {
            Vec2  new_center = center;
            Float new_radius = radius / 2;

            switch (quad) {
                case QUAD_NE:
                    new_center.x += new_radius;
                    new_center.y += new_radius;
                    break;
                case QUAD_SE:
                    new_center.x += new_radius;
                    new_center.y -= new_radius;
                    break;
                case QUAD_NW:
                    new_center.x -= new_radius;
                    new_center.y += new_radius;
                    break;
                case QUAD_SW:
                    new_center.x -= new_radius;
                    new_center.y -= new_radius;
                    break;
            }

            return new_center;
        }

        static void impl_apply_gravity_target_source(Body& target_body, const PointMass& source_pm,
                                                     Float g, Float softening) {
            Vec2  delta               = source_pm.pos.sub(target_body.pm.pos);
            Float r2_soft             = delta.length_sq() + (softening * softening);
            Float inv_r3              = 1.0 / (std::sqrt(r2_soft) * r2_soft);
            Vec2  source_contribution = delta.scale(g * source_pm.mass * inv_r3);
            target_body.acc           = target_body.acc.add(source_contribution);
        }
    };
};
}  // namespace nbody2
