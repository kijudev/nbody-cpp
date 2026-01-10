#pragma once

#include <concepts>
#include <limits>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// TODO: Move to some other file.
template <typename MortonCode>
concept MortonCodeT = std::same_as<MortonCode, U32> || std::same_as<MortonCode, U64>;

template <FloatT Float, MortonCodeT MortonCode>
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

    // --- Morton Quad Tree Typedefs ---
    using QuadId                    = U8;
    static constexpr QuadId QUAD_NE = 3;
    static constexpr QuadId QUAD_NW = 2;
    static constexpr QuadId QUAD_SE = 1;
    static constexpr QuadId QUAD_SW = 0;

   public:
    // --- Public Interface ---

    // NOTE: The only public constructor; takes in the config struct.
    BarnesHutMorton(const Config& config);

    // NOTE: Returns the bodies present in the current simulation.
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;

    // NOTE: Inserts a body at the end of the bodies vector inside the simulation class.
    void insert_body(Body&& body);

    // NOTE: Proceeds one step in the simulation.
    void step(Float dt);

   private:
    // --- Inline Node ---
    struct InlineNode {
        PointMass com{
            {0.0, 0.0},
            {0.0},
        };
        MortonCode morton_code{0};
    };

    // --- Member Variables ---
    const IntegrateBodyFnT<Float> m_integrate{integrate_body_euler<Float>};
    const Float                   m_g{DEFAULT_G};
    const Float                   m_softening{DEFAULT_SOFTENING};
    const Float                   m_theta{DEFAULT_THETA};

    std::vector<Body>       m_bodies{};
    std::vector<InlineNode> m_nodes{};
    Vec2                    m_root_center{0.0, 0.0};
    Float                   m_root_radius{1.0};
    std::vector<bool>       m_bodies_lookup{};
    USize                   m_dead_bodies_count{0};
    USize                   m_dead_bodies_next{DEAD_BODY_NULL};

    // --- Implementation ---
    void impl_create_tree();
    void impl_compute_acceleration_all();
    void impl_integrate_all();
};
}  // namespace nbody::sim
