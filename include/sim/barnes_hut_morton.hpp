#pragma once

#include <span>
#include <vector>

#include "base/type.hpp"
#include "math/morton.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float, math::MortonCodeT MortonCode = U64>
class BarnesHutMorton : public SimInterface<Float> {
   public:
    // --- General Typedefs ---
    using Vec2 = math::Vec2T<Float>;
    using Body = BodyT<Float>;

    // --- Morton Code Constants ---
    static constexpr USize MORTON_BITS  = sizeof(MortonCode) * 8;
    static constexpr USize BITS_PER_DIM = MORTON_BITS / 2;
    static constexpr USize MAX_DEPTH    = BITS_PER_DIM;

    struct Node {
        MortonCode prefix;          // Morton code prefix identifying this node's spatial region
        U8         level;           // Tree level (0 = root, MAX_DEPTH = leaves)
        Vec2       center_of_mass;  // Center of mass of all bodies in this subtree
        Float      total_mass;      // Total mass of all bodies in this subtree
        Float      size;            // Spatial extent (side length) of this node's region
        USize      first_idx;       // First body index (inclusive) in sorted array
        USize      last_idx;        // Last body index (exclusive) in sorted array
        USize      children[4];     // Indices of child nodes (NODE_EMPTY if no child)

        static constexpr USize NODE_EMPTY = std::numeric_limits<USize>::max();

        [[nodiscard]] bool is_leaf() const {
            return children[0] == NODE_EMPTY && children[1] == NODE_EMPTY &&
                   children[2] == NODE_EMPTY && children[3] == NODE_EMPTY;
        }

        [[nodiscard]] USize body_count() const { return last_idx - first_idx; }
    };

    struct MortonBody {
        MortonCode morton;
        USize      body_idx;

        bool operator<(const MortonBody& other) const { return morton < other.morton; }
    };

    // --- Config ---
    struct Config {
        std::vector<Body>       bodies{};
        Float                   g{sim::scale_toy::G};
        Float                   softening{sim::scale_toy::SOFTENING};
        Float                   theta{0.5};
        bool                    parallel{false};
        bool                    radix{false};
        IntegrateBodyFnT<Float> integrate_fn{integrate_body_euler<Float>};
    };

    // --- Public Interface ---
    explicit BarnesHutMorton(const Config& config);

    void step(Float dt) override;
    void insert(Body&& body);

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const override;
    [[nodiscard]] const std::vector<Node>&                   nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<MortonBody>& morton_bodies() const { return m_morton_bodies; }

   private:
    // --- Tree Construction ---
    void build_tree();
    void compute_morton_codes();
    void sort_by_morton();
    void build_nodes_recursive(USize first, USize last, U8 level, MortonCode prefix,
                               Float node_size, Vec2 node_center);
    void compute_node_properties(USize node_idx);

    // --- Force Calculation ---
    Vec2 compute_acceleration(USize body_idx) const;
    Vec2 compute_acceleration_from_node(USize node_idx, Vec2 pos) const;

    // --- Utility ---
    [[nodiscard]] Vec2  get_node_center_from_prefix(MortonCode prefix, U8 level) const;
    [[nodiscard]] Float get_node_size(U8 level) const;
    [[nodiscard]] U8    get_quadrant(MortonCode code, U8 level) const;

    // --- Member Variables ---
    std::vector<Body>       m_bodies;
    std::vector<MortonBody> m_morton_bodies;
    std::vector<Node>       m_nodes;

    Float m_g;
    Float m_softening;
    Float m_theta;
    bool  m_parallel;
    bool  m_radix;

    Float m_bounds_min;
    Float m_bounds_max;

    IntegrateBodyFnT<Float> m_integrate_fn;
};
}  // namespace nbody::sim
