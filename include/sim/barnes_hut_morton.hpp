#pragma once

#include <array>
#include <limits>
#include <span>
#include <vector>

#include "base/type.hpp"
#include "math/morton.hpp"
#include "math/vec.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float, math::MortonCodeT MortonCode>
class BarnesHutMorton {
   public:
    // --- General Typedefs ---
    using Vec2            = math::Vec2T<Float>;
    using Body            = BodyT<Float>;
    using PointMass       = PointMassT<Float>;
    using IntegrateBodyFn = IntegrateBodyFnT<Float>;

    // --- Deafults ---
    static constexpr Float DEFAULT_G         = sim::scale_toy::G;
    static constexpr Float DEFAULT_SOFTENING = sim::scale_toy::SOFTENING;
    static constexpr Float DEFAULT_THETA     = 0.5;
    static constexpr Float MIN_ROOT_RADIUS   = 1.0;
    static constexpr USize DEAD_BODY_NULL    = std::numeric_limits<USize>::max();

    // --- Config ---
    struct Config {
        std::vector<Body> bodies{};
        IntegrateBodyFn   integrate_fn{integrate_body_euler<Float>};
        Float             g{DEFAULT_G};
        Float             softening{DEFAULT_SOFTENING};
        Float             theta{0.5};
    };

    // --- Public Interface ---

    // NOTE: The only public constructor; takes in the config struct.
    BarnesHutMorton(const Config& config);

    // NOTE: Returns the bodies present in the current simulation.
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;

    // NOTE: Inserts a body at the end of the bodies vector inside the simulation class.
    void insert_body(Body&& body);

    // NOTE: Proceeds one step in the simulation.
    void step(Float dt);

   public:
    // --- Morton Quad Tree Typedefs ---
    using QuadId                       = U8;
    static constexpr QuadId QUAD_NE    = 3;
    static constexpr QuadId QUAD_NW    = 2;
    static constexpr QuadId QUAD_SE    = 1;
    static constexpr QuadId QUAD_SW    = 0;
    static constexpr USize  NODE_EMPTY = std::numeric_limits<USize>::max();
    static constexpr USize  NODE_ROOT  = 0;
    static constexpr USize  MAX_DEPTH  = sizeof(MortonCode) == 4 ? 16 : 32;

    // --- Quad Tree Node ---
    struct Node {
        MortonCode code{0};
        Vec2       pos{0.0, 0.0};
        Float      mass{0.0};
        Float      quad_radius{0.0};

        std::array<USize, 4> children{
            NODE_EMPTY,
            NODE_EMPTY,
            NODE_EMPTY,
            NODE_EMPTY,
        };
    };

   public:
    // --- Member Variables ---
    const IntegrateBodyFnT<Float> m_integrate{integrate_body_euler<Float>};
    const Float                   m_g{DEFAULT_G};
    const Float                   m_softening{DEFAULT_SOFTENING};
    const Float                   m_theta{DEFAULT_THETA};

    std::vector<Body>                        m_bodies{};
    std::vector<std::pair<Body, MortonCode>> m_bodies_sorted{};
    std::vector<Node>                        m_nodes{};
    Vec2                                     m_root_center{0.0, 0.0};
    Float                                    m_root_radius{1.0};
    std::vector<bool>                        m_bodies_lookup{};
    USize                                    m_dead_bodies_count{0};
    USize                                    m_dead_bodies_next{DEAD_BODY_NULL};

    // --- Implementation ---
    void impl_reset_acceleration_all();
    void impl_create_tree();
    void impl_compute_acceleration_all();
    void impl_integrate_all(Float dt);

    void impl_compute_root_center_radius();
    void impl_compute_bodies_sorted();

    // --- Tree Construction and Traversal ---
    void   impl_insert_body_to_tree(const PointMass& insert_pm, MortonCode insert_code,
                                    Float insert_quad_radius);
    QuadId impl_get_quad_from_morton(MortonCode code, USize level) const;
    void   impl_compute_node_mass_recursive(USize node_idx);
    void   impl_compute_acceleration_body_node(Body& body, USize node_idx);
    void   impl_compute_acceleration_body_source(Body& body, const Vec2& source_pos,
                                                 Float source_mass);
    bool   impl_is_node_leaf(const Node& node) const;
};
}  // namespace nbody::sim
