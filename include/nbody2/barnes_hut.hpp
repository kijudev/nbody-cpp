#pragma once

#include <cmath>
#include <memory>
#include <span>

#include "base/type.hpp"
#include "type.hpp"

namespace nbody2 {
template <FloatT Float>
class SimDirect {
   public:
    using Vec2        = math::Vec2T<Float>;
    using Body        = BodyT<Float>;
    using Layout      = std::vector<Body>;
    using IntegrateFn = IntegrateFn<Float>;
    using Quad        = USize;

    static constexpr Quad QUAD_NE = 0;
    static constexpr Quad QUAD_NW = 1;
    static constexpr Quad QUAD_SE = 2;
    static constexpr Quad QUAD_SW = 3;

    struct NodeBody;
    enum class NodeKind;
    struct Node;

    SimDirect(Layout bodies, IntegrateFn integrator, Float g, Float softening)
        : m_bodies(std::move(bodies)),
          m_integrate(std::move(integrator)),
          m_g(g),
          m_softening(softening) {}

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const { return m_bodies; }
    void add_body(const Body& body) { m_bodies.push_back(body); }

    void step(Float dt) {
        for (Body& body : m_bodies) {
            body.acc = Vec2::zero();
        }

        apply_gravity();

        for (Body& body : m_bodies) {
            m_integrate(body, dt);
        }
    }

   private:
    Layout                m_bodies{};
    std::unique_ptr<Node> m_root{nullptr};

    const IntegrateFn m_integrate{};
    const Float       m_g{};
    const Float       m_softening{};

    void apply_gravity() {
        // TODO
    }

    void construct_tree() {
        init_root();

        for (const Body& body : m_bodies) {
            m_root->insert(NodeBody::make_from_body(body));
        }
    }

    void init_root() {
        if (m_bodies.empty()) {
            m_root = make_internal(Vec2::zero(), 0.0);
            return;
        }

        Float max_x = std::numeric_limits<Float>::lowest();
        Float max_y = std::numeric_limits<Float>::lowest();
        Float min_x = std::numeric_limits<Float>::max();
        Float min_y = std::numeric_limits<Float>::max();

        for (const Body& body : m_bodies) {
            max_x = std::max(max_x, body.pos.x);
            max_y = std::max(max_y, body.pos.y);
            min_x = std::min(min_x, body.pos.x);
            min_y = std::min(min_y, body.pos.y);
        }

        Float width  = max_x - min_x;
        Float height = max_y - min_y;

        Float radius = std::max(width, height) / 2.0;
        Vec2  center = Vec2{min_x + width / 2.0, min_y + height / 2.0};

        m_root = make_internal(center, radius);
    }

   public:
    struct NodeBody {
        Vec2  pos{0.0, 0.0};
        Float mass{1.0};

        static NodeBody make_from_body(const Body& body) { return NodeBody{body.pos, body.mass}; }
    };

    enum class NodeKind { INTERNAL, EXTERNAL };

    struct Node {
        Vec2     center{0.0, 0.0};
        Float    mass{0.0};
        Float    radius{0.0};
        NodeKind kind{NodeKind::INTERNAL};
        Vec2     body_pos{0.0, 0.0};

        std::array<std::unique_ptr<Node>, 4> children{nullptr, nullptr, nullptr, nullptr};

        static std::unique_ptr<Node> make_internal(const Vec2& center, Float radius) {
            return std::make_unique<Node>(center, 0.0, radius, NodeKind::INTERNAL, center);
        }

        static std::unique_ptr<Node> make_external(const Vec2& center, Float radius,
                                                   const Body& body) {
            return std::make_unique<Node>(center, body.mass, radius, NodeKind::EXTERNAL, body.pos);
        }

        bool is_internal() const { return kind == NodeKind::INTERNAL; }
        bool is_external() const { return kind == NodeKind::EXTERNAL; }

        NodeBody extract_node_body() const {
            ASSERT(is_external());
            return NodeBody{body_pos, mass};
        }

        Quad find_quad_for_pos(const Vec2& pos) const {
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

        void update_mass(const NodeBody& body) {
            ASSERT(is_internal());
            mass += body.mass;
        }

        void update_com(const NodeBody& body) {
            ASSERT(is_internal());
            center = center.scale(mass).add(body.pos.scale(body.mass));
            center = center.scale(1 / (mass + body.mass));
        }

        Vec2 get_center_of_quad(Quad quad) const {
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

        void insert_body(const NodeBody& body) {
            Quad quad = find_quad_for_pos(body.pos);

            if (!children[quad]) {
                children[quad] = make_external(get_center_of_quad(quad), radius / 2, body);
            } else if (is_internal()) {
                children[quad]->insert_body(body);
            } else {
                Quad node_body_quad = find_quad_for_pos(body_pos);

                kind = NodeKind::INTERNAL;
                children[node_body_quad] =
                    make_external(get_center_of_quad(node_body_quad), radius / 2, body);

                insert_body(body);
            }

            update_mass(body);
            update_com(body);
        }

       private:
        Node() = default;
        Node(const Vec2& center_, const Vec2& mass_, Float radius_, NodeKind kind_,
             const Vec2& body_pos_)
            : center(center_), mass(mass_), radius(radius_), kind(kind_), body_pos(body_pos_) {}
    };
};
}  // namespace nbody2
