#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <span>
#include <vector>

#include "base/assert.hpp"
#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class BarnesHut {
   public:
    // NOTE: Type aliases; for convenience.
    using Vec2            = math::Vec2T<Float>;
    using Body            = BodyT<Float>;
    using PointMass       = PointMassT<Float>;
    using Layout          = std::vector<Body>;
    using IntegrateBodyFn = IntegrateBodyFnT<Float>;

    // NOTE: Quad info. Public; to be used by the renderer.
    using QuadId                                   = USize;
    static constexpr QuadId QUAD_NE                = 0;
    static constexpr QuadId QUAD_NW                = 1;
    static constexpr QuadId QUAD_SE                = 2;
    static constexpr QuadId QUAD_SW                = 3;
    static constexpr Float  M_MIN_ROOT_QUAD_RADIUS = 1.0;

    enum class NodeKind { EMPTY, REGION, LEAF };

    class Node {
       public:
        Vec2                                 quad_center{0.0, 0.0};
        Float                                quad_radius{0.0};
        Vec2                                 center{0.0, 0.0};
        Float                                mass{0.0};
        NodeKind                             kind{NodeKind::EMPTY};
        std::array<std::unique_ptr<Node>, 4> children{nullptr, nullptr, nullptr, nullptr};

       public:
        static std::unique_ptr<Node> make_ptr_empty(const Vec2& qc, Float qr);
        static std::unique_ptr<Node> make_ptr_region(const Vec2& qc, Float qr);
        static std::unique_ptr<Node> make_ptr_leaf(const Vec2& qc, Float qr, const PointMass& pm);

        bool is_empty() const;
        bool is_region() const;
        bool is_leaf() const;
        void insert_point_mass(const PointMass& pm);
        void apply_gravity_body(Body& body, Float g, Float softening, Float theta) const;

       private:
        Node() = default;
        Node(const Vec2& quad_center_, Float quad_radius_, const Vec2& center_, Float mass_,
             NodeKind kind_);

        PointMass self_as_point_mass() const;
        void      self_recompute_com_mass();
        QuadId    impl_pos_quad_id(const Vec2& pos) const;
        Vec2      impl_quad_id_center(QuadId quad_id) const;
        void      impl_apply_gravity_body_source(Body& body, const PointMass& pm, Float g,
                                                 Float softening);
    };

    struct Config {
        Layout          bodies{};
        IntegrateBodyFn integrate_fn{integrate_body_euler<Float>};
        Float           g{G_TOY};
        Float           softening{SOFTENING_TOY};
        Float           theta{0.5};
    };

   public:
    BarnesHut(const Config& config);

   public:
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;
    void                                                     insert_body(Body&& body);
    void                                                     step(Float dt);

    // Return a flat list of node pointers (const) for the renderer. Null root returns empty list.
    std::vector<const Node*> collect_nodes() const {
        if (!m_root) {
            return {};
        }

        std::vector<const NodeOld*> out;
        out.reserve(64);

        std::vector<const NodeOld*> stack;
        stack.push_back(m_root.get());

        while (!stack.empty()) {
            const NodeOld* node = stack.back();
            stack.pop_back();

            out.push_back(node);

            if (node->is_region()) {
                // push children if present
                for (const auto& child : node->children) {
                    if (child) {
                        stack.push_back(child.get());
                    }
                }
            }
        }

        return out;
    }

   private:
    Layout                m_bodies{};       // NOTE: Stores the bodies in the simulation.
    std::unique_ptr<Node> m_root{nullptr};  // NOTE: Stores the Barnes-Hut quadtree.

    const IntegrateBodyFn m_integrate{
        integrate_body_euler<Float>};  // NOTE: Integrates the bodies' positions and velocities.
    const Float m_g{G_TOY};            // NOTE: Gravitational constant. can be changed at runtime.
    Float m_softening{SOFTENING_TOY};  // NOTE: Softening parameter; can be changed at runtime.
    Float m_theta{0.5};                // NOTE: Theta parameter for Barnes-Hut approximation.

    // NOTE: Applies gravity to each body.
    void impl_apply_gravity() {
        if (!m_root) {
            return;
        }

        for (Body& target_body : m_bodies) {
            m_root->apply_gravity_target(target_body, m_g, m_softening);
        }
    }

    // NOTE: Constructs the Barnes-Hut quadtree.
    void impl_construct_tree() {
        impl_init_root();

        for (const Body& body : m_bodies) {
            if (m_root) {
                m_root->insert_point_mass(body.pm);
            }
        }
    }

    // NOTE: Initializes the root node of the Barnes-Hut quadtree.
    void impl_init_root() {
        if (m_bodies.empty()) {
            m_root = Node::make_region(Vec2::make_zero(), 0.0);
            return;
        }

        auto [center, radius] = impl_root_node_center_radius();

        m_root = Node::make_region(center, radius);
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
    struct NodeOld {
       public:
        Vec2  region_center{0.0, 0.0};
        Float region_radius{0.0};

        // NOTE:
        // A) If the node is EMPTY, it the values should be set to zero.
        // B) If the node is REGION, com is the center of mass of the region, and mass is the total
        // mass of the region.
        // C) If the node is LEAF, com is the position of the point mass, and
        // mass is the mass of the point mass.
        Vec2  com{0.0, 0.0};
        Float mass{0.0};

        NodeKind kind{NodeKind::EMPTY};

        std::array<std::unique_ptr<NodeOld>, 4> children{};

        static std::unique_ptr<NodeOld> make_empty(const Vec2& center, Float radius) {
            std::unique_ptr<NodeOld> node = std::make_unique<NodeOld>();
            node->region_center           = center;
            node->region_radius           = radius;
            node->kind                    = NodeKind::EMPTY;

            return node;
        }

        static std::unique_ptr<NodeOld> make_region(const Vec2& center, Float radius) {
            std::unique_ptr<NodeOld> node = std::make_unique<NodeOld>();
            node->region_center           = center;
            node->region_radius           = radius;
            node->kind                    = NodeKind::REGION;

            return node;
        }

        static std::unique_ptr<NodeOld> make_leaf(const Vec2& center, Float radius,
                                                  const PointMass& pm) {
            std::unique_ptr<NodeOld> node = std::make_unique<NodeOld>();
            node->region_center           = center;
            node->region_radius           = radius;
            node->com                     = pm.pos;
            node->mass                    = pm.mass;
            node->kind                    = NodeKind::LEAF;

            return node;
        }

        // Insert a point mass into the quadtree rooted at this node.
        void insert_point_mass(const PointMass& pm) {
            if (is_empty()) {
                kind = NodeKind::LEAF;
                com  = pm.pos;
                mass = pm.mass;
            } else if (is_region()) {
                QuadId q = impl_pos_quad(pm.pos);

                if (!children[q]) {
                    children[q] = make_leaf(impl_quad_center(q), region_radius / 2, pm);
                } else {
                    children[q]->insert_point_mass(pm);
                }

                // NOTE: Recompute mass & com by aggregating children.
                self_recompute_com_mass();
            } else if (is_leaf()) {
                PointMass self_pm = point_mass();

                kind = NodeKind::REGION;

                for (USize q = 0; q < 4; ++q) {
                    children[q] = make_empty(impl_quad_center(q), region_radius / 2);
                }

                // NOTE: Insert self as point mass into the appropriate child.
                QuadId self_quad = impl_pos_quad(self_pm.pos);
                children[self_quad]->insert_point_mass(self_pm);

                // NOTE: Insert the new point mass into the appropriate child.
                QuadId new_quad = impl_pos_quad(pm.pos);
                children[new_quad]->insert_point_mass(pm);

                // NOTE: Recompute mass & com by aggregating children.
                self_recompute_com_mass();
            }
        }

        void apply_gravity_target(Body& target, Float g, Float softening) const {
            if (is_leaf()) {
                if (mass > 0.0) {
                    impl_apply_gravity_target_source(target, point_mass(), g, softening);
                }

                return;
            }

            Float constexpr theta = static_cast<Float>(0.5);

            Float s     = region_radius * 2.0;
            Vec2  delta = com.sub(target.pm.pos);
            Float dist  = delta.length();

            if (dist <= std::numeric_limits<Float>::epsilon()) {
                return;
            }

            Float ratio = s / dist;
            if (ratio < theta) {
                impl_apply_gravity_target_source(target, point_mass(), g, softening);
            } else {
                for (const std::unique_ptr<NodeOld>& child : children) {
                    if (child) {
                        child->apply_gravity_target(target, g, softening);
                    }
                }
            }
        }

        bool is_empty() const { return kind == NodeKind::EMPTY; }
        bool is_region() const { return kind == NodeKind::REGION; }
        bool is_leaf() const { return kind == NodeKind::LEAF; }

        PointMass point_mass() const {
            // NOTE: This assertion should be used to ensure that the node contains a single point
            // mass. But it can randomly fail due to memory corruption, or use after frees, and
            // other shenanigans.
            // ASSERT(is_leaf(), "Node is not a leaf");
            return {.pos = com, .mass = mass};
        }

        std::string fmt() const {
            return std::format(
                "Node{{ region_center: {}, region_radius: {}, mass: {}, com: {}, kind: {}}}",
                region_center.to_string(), std::to_string(region_radius), std::to_string(mass),
                com.to_string());
        }

       private:
        void self_recompute_com_mass() {
            ASSERT(!is_empty(), "Node is empty");

            Float total_mass   = static_cast<Float>(0.0);
            Vec2  weighted_sum = Vec2::make_zero();

            for (const auto& child : children) {
                if (child && child->mass > static_cast<Float>(0.0)) {
                    weighted_sum = weighted_sum.add(child->com.scale(child->mass));
                    total_mass += child->mass;
                }
            }

            if (total_mass > 0.0) {
                com  = weighted_sum.scale(1.0 / total_mass);
                mass = total_mass;
            } else {
                com  = Vec2::make_zero();
                mass = 0.0;
            }
        }

        QuadId impl_pos_quad(const Vec2& pos) const {
            if (pos.x >= region_center.x) {
                if (pos.y >= region_center.y) {
                    return QUAD_NE;
                } else {
                    return QUAD_SE;
                }
            } else {
                if (pos.y >= region_center.y) {
                    return QUAD_NW;
                } else {
                    return QUAD_SW;
                }
            }
        }

        Vec2 impl_quad_center(QuadId quad) const {
            Vec2  new_center = region_center;
            Float new_radius = region_radius / 2.0;

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
            Vec2  delta   = source_pm.pos.sub(target_body.pm.pos);
            Float r2_soft = delta.length_sq() + (softening * softening);

            if (r2_soft <= std::numeric_limits<Float>::epsilon()) {
                return;
            }

            Float inv_r3              = 1.0 / (std::sqrt(r2_soft) * r2_soft);
            Vec2  source_contribution = delta.scale(g * source_pm.mass * inv_r3);
            target_body.acc           = target_body.acc.add(source_contribution);
        }
    };
};
}  // namespace nbody::sim
