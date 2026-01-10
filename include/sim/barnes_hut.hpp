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
    // --- General Typedefs ---
    using Vec2            = math::Vec2T<Float>;
    using Body            = BodyT<Float>;
    using PointMass       = PointMassT<Float>;
    using Layout          = std::vector<Body>;
    using IntegrateBodyFn = IntegrateBodyFnT<Float>;

    // --- Quad Tree Typedefs ---
    using QuadId                                   = USize;
    static constexpr QuadId QUAD_NE                = 0;
    static constexpr QuadId QUAD_NW                = 1;
    static constexpr QuadId QUAD_SE                = 2;
    static constexpr QuadId QUAD_SW                = 3;
    static constexpr Float  M_MIN_ROOT_QUAD_RADIUS = 1.0;
    enum class NodeKind { EMPTY, REGION, LEAF };
    class Node;

    // --- Defaults ---
    static constexpr Float DEFAULT_G         = sim::scale_toy::G;
    static constexpr Float DEFAULT_SOFTENING = sim::scale_toy::SOFTENING;

    // --- Config ---
    // NOTE: The Config struct is used to configure the BarnesHut simulation. It can be the one and
    // only parameter to the contructor of the class.
    // NOTE: Params:
    // - bodies: vector of bodies provided as a starting point to the simulation.
    // - integrate_fn: function used to numerically integrate the gravitational force.
    // - g: gravitational constant.
    // - softening: softening parameter ment to dampen the force when the bodies are close together.
    // - theta: the theta parameter for the Barnes-Hut algorithm.
    // - depth: max recursion depth for the Quad Tree construction. Point masses are joint together
    // into one quad if the recursion hits this limit.
    struct Config {
        Layout          bodies{};
        IntegrateBodyFn integrate_fn{integrate_body_euler<Float>};
        Float           g{DEFAULT_G};
        Float           softening{DEFAULT_SOFTENING};
        Float           theta{0.5};
        U16             depth{64};
    };

   public:
    BarnesHut(const Config& config);

   public:
    // NOTE: Returns the bodies present in the current simulation.
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;

    // NOTE: Inserts a body at the end of the bodies vector inside the simulation class.
    void insert_body(Body&& body);

    // NOTE: Proceeds one step in the simulation.
    void step(Float dt);

    // NOTE: Collects all of the Quad Tree nodes into a vector of const pointers.
    [[nodiscard]] std::vector<const Node*> collect_nodes() const;

   private:
    Layout                m_bodies{};       // NOTE: Stores the bodies in the simulation.
    std::unique_ptr<Node> m_root{nullptr};  // NOTE: Stores the Barnes-Hut quadtree.

    const IntegrateBodyFn m_integrate{
        integrate_body_euler<Float>};  // NOTE: Integrates the bodies' positions and velocities.
    const Float m_g{DEFAULT_G};        // NOTE: Gravitational constant. can be changed at runtime.
    Float m_softening{DEFAULT_SOFTENING};  // NOTE: Softening parameter; can be changed at runtime.
    Float m_theta{0.5};                    // NOTE: Theta parameter for Barnes-Hut approximation.
    U16   m_depth{64};

    void impl_create_root();
    void impl_construct_tree();

    // NOTE: Applies the gravitational force (calculates the acceleration) to all the bodies in the
    // simulation.
    void impl_apply_gravity();

    // NOTE: Calculates the radius of the root node. The radius of the root node is half the side
    // length of the smallest bounding box containing all of the bodies in the simulation.
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

        // WARNING: Be carefuf while using outside of the simulation class.
        void insert_point_mass(const PointMass& pm, U16 current_depth, U16 max_depth);

        // WARNING: Be carefuf while using outside of the simulation class.
        void apply_gravity_body(Body& body, Float g, Float softening, Float theta) const;

        std::string to_string() const;

        Node() = default;

       private:
        PointMass self_as_point_mass() const;
        void      self_recompute_com_mass();
        QuadId    impl_pos_quad_id(const Vec2& pos) const;
        Vec2      impl_quad_id_center(QuadId quad_id) const;

        // NOTE: Applies the gravitational force (calculates the acceleration) to the body from the
        // point mass. Where the point mass could be another body all the aproximate point mass of
        // the quad.
        void impl_apply_gravity_body_source(Body& body, const PointMass& pm, Float g,
                                            Float softening) const;
    };
};
}  // namespace nbody::sim
