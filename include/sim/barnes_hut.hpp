#pragma once

#include <cmath>
#include <memory>
#include <span>
#include <vector>

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
    class Node;

    struct Config {
        Layout          bodies{};
        IntegrateBodyFn integrate_fn{integrate_body_euler<Float>};
        Float           g{G_TOY};
        Float           softening{SOFTENING_TOY};
        Float           theta{0.5};
        U16             depth{64};
    };

   public:
    BarnesHut(const Config& config);

   public:
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;
    void                                                     insert_body(Body&& body);
    void                                                     step(Float dt);
    [[nodiscard]] std::vector<const Node*>                   collect_nodes() const;

   private:
    Layout                m_bodies{};       // NOTE: Stores the bodies in the simulation.
    std::unique_ptr<Node> m_root{nullptr};  // NOTE: Stores the Barnes-Hut quadtree.

    const IntegrateBodyFn m_integrate{
        integrate_body_euler<Float>};  // NOTE: Integrates the bodies' positions and velocities.
    const Float m_g{G_TOY};            // NOTE: Gravitational constant. can be changed at runtime.
    Float m_softening{SOFTENING_TOY};  // NOTE: Softening parameter; can be changed at runtime.
    Float m_theta{0.5};                // NOTE: Theta parameter for Barnes-Hut approximation.
    U16   m_depth{64};

    void                    impl_create_root();
    void                    impl_construct_tree();
    void                    impl_apply_gravity();
    std::tuple<Vec2, Float> impl_root_node_center_radius();

   public:
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
        void insert_point_mass(const PointMass& pm, U16 current_depth, U16 max_depth);
        void apply_gravity_body(Body& body, Float g, Float softening, Float theta) const;

        std::string to_string() const;

        Node() = default;

       private:
        PointMass self_as_point_mass() const;
        void      self_recompute_com_mass();
        QuadId    impl_pos_quad_id(const Vec2& pos) const;
        Vec2      impl_quad_id_center(QuadId quad_id) const;
        void      impl_apply_gravity_body_source(Body& body, const PointMass& pm, Float g,
                                                 Float softening) const;
    };
};
}  // namespace nbody::sim
