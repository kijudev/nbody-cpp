#pragma once

#include <array>
#include <limits>
#include <span>
#include <tuple>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class BarnesHutLinear {
   public:
    using Vec2            = math::Vec2T<Float>;
    using Body            = BodyT<Float>;
    using PointMass       = PointMassT<Float>;
    using Layout          = std::vector<Body>;
    using IntegrateBodyFn = IntegrateBodyFnT<Float>;

    using QuadId                                   = USize;
    static constexpr QuadId QUAD_NE                = 0;
    static constexpr QuadId QUAD_NW                = 1;
    static constexpr QuadId QUAD_SE                = 2;
    static constexpr QuadId QUAD_SW                = 3;
    static constexpr Float  M_MIN_ROOT_QUAD_RADIUS = 1.0;

    enum class NodeKind { EMPTY, REGION, LEAF };

    static constexpr Float DEAFULT_G         = sim::scale_toy::G;
    static constexpr Float DEAFULT_SOFTENING = sim::scale_toy::SOFTENING;

    struct Node {
        Vec2     quad_center{0.0, 0.0};  // NOTE: Geometric center of the quad.
        Float    quad_radius{0.0};       // NOTE: Galf-side-length (radius).
        Vec2     center{0.0, 0.0};       // NOTE: Center-of-mass for this node (if region/leaf).
        Float    mass{0.0};              // NOTE: Total mass represented by this node.
        NodeKind kind{NodeKind::EMPTY};

        std::array<USize, 4> children{
            std::numeric_limits<USize>::max(),
            std::numeric_limits<USize>::max(),
            std::numeric_limits<USize>::max(),
            std::numeric_limits<USize>::max(),
        };

        bool is_empty() const noexcept { return kind == NodeKind::EMPTY; }
        bool is_region() const noexcept { return kind == NodeKind::REGION; }
        bool is_leaf() const noexcept { return kind == NodeKind::LEAF; }

        PointMass self_as_point_mass() const { return PointMass{.pos = center, .mass = mass}; }
    };

    struct Config {
        Layout          bodies{};
        IntegrateBodyFn integrate_fn{integrate_body_euler<Float>};
        Float           g{DEAFULT_G};
        Float           softening{DEAFULT_SOFTENING};
        Float           theta{0.5};
        U16             depth{64};

        // NOTE: Reserve approximate number of nodes to reduce reallocs.
        USize reserve_nodes{0};
    };

   public:
    BarnesHutLinear(const Config& config);

   public:
    void step(Float dt);

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;
    void                                                     insert_body(Body&& body);
    std::vector<USize>                                       collect_node_indices() const;
    const std::vector<Node>&                                 nodes() const { return m_nodes; }

   private:
    Layout            m_bodies{};
    std::vector<Node> m_nodes{};
    USize             m_next_free{0};  // NOTE: Next free index when using pre-sized node pool.
    USize m_root_index{std::numeric_limits<USize>::max()};  // NOTE: Index of root node, or -1.

    const IntegrateBodyFn m_integrate{integrate_body_euler<Float>};
    const Float           m_g{DEAFULT_G};
    Float                 m_softening{DEAFULT_SOFTENING};
    Float                 m_theta{0.5};
    U16                   m_depth{64};

    USize node_make_empty(const Vec2& qc, Float qr);
    USize node_make_region(const Vec2& qc, Float qr);
    USize node_make_leaf(const Vec2& qc, Float qr, const PointMass& pm);

    void impl_create_root();
    void impl_construct_tree();
    void impl_apply_gravity();

    void node_insert_point_mass(USize node_idx, const PointMass& pm, U16 current_depth,
                                U16 max_depth);

    void node_self_recompute_com_mass(USize node_idx);

    QuadId node_impl_pos_quad_id(USize node_idx, const Vec2& pos) const;

    Vec2 node_impl_quad_id_center(USize node_idx, QuadId qid) const;

    void node_impl_apply_gravity_body_source(Body& body, const PointMass& pm, Float g,
                                             Float softening) const;

    void node_apply_gravity_at(USize node_idx, Body& body, Float g, Float softening,
                               Float theta) const;

    // NOTE: Root bounding calculation.
    std::tuple<Vec2, Float> impl_root_node_center_radius() const;
};
}  // namespace nbody::sim
