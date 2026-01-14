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

template <FloatT Float, math::MortonCodeT MortonCode = U64>
class BarnesHutMorton : public SimInterface<Float> {
   public:
    // --- General Typedefs ---
    using Vec2      = math::Vec2T<Float>;
    using Body      = BodyT<Float>;
    using NodeIndex = USize;

    // --- Morton Code Constants ---
    static constexpr NodeIndex NODE_INDEX_NULL =
        std::numeric_limits<NodeIndex>::max();
    static constexpr USize MORTON_BITS  = sizeof(MortonCode) * 8;
    static constexpr USize BITS_PER_DIM = MORTON_BITS / 2;
    static constexpr USize MAX_DEPTH    = BITS_PER_DIM;

    // NOTE: Body index paired with its Morton code for sorting.
    struct BodyCode {
        USize      index;
        MortonCode code;
    };

    // NOTE: Tree node representing either a leaf (single body) or internal node
    // (aggregate). For internal nodes, children[i] indexes the child in
    // quadrant i (0-3). For leaf nodes, body_index points to the body in
    // m_bodies. The `level` field indicates the tree depth (0 = root, MAX_DEPTH
    // = finest).
    struct Node {
        std::array<NodeIndex, 4> children{NODE_INDEX_NULL, NODE_INDEX_NULL,
                                          NODE_INDEX_NULL, NODE_INDEX_NULL};
        NodeIndex                parent{NODE_INDEX_NULL};
        MortonCode prefix;  // Morton code prefix for this node's region
        USize      level;   // Tree level (0 = root)
        Vec2       com;     // Center of mass
        Float      mass;    // Total mass
        Float      size;    // Spatial size of this node's region
        bool       is_leaf{false};
        USize      body_index{0};  // Valid only if is_leaf

        [[nodiscard]] bool is_empty() const { return mass == 0.0; }
        [[nodiscard]] bool is_internal() const {
            return !is_leaf && children[0] != NODE_INDEX_NULL;
        }
    };

    // --- Config ---
    struct Config {
        std::vector<Body>       bodies{};
        Float                   g{sim::scale_toy::G};
        Float                   softening{sim::scale_toy::SOFTENING};
        Float                   theta{0.5};
        bool                    parallel{false};
        IntegrateBodyFnT<Float> integrate_fn{integrate_body_euler<Float>};
    };

    // --- Public Interface ---
    explicit BarnesHutMorton(const Config& config = Config{});

    void step(Float dt) override;
    void insert_body(Body&& body) override;

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies()
        const override;
    [[nodiscard]] const std::vector<Node>& nodes() const;

   private:
    // --- Data ---
    std::vector<Body>     m_bodies;
    std::vector<BodyCode> m_codes;
    std::vector<Node>     m_nodes;

    Float m_extent_min{0.0};
    Float m_extent_max{0.0};
    Float m_root_size{0.0};

    Float                   m_g;
    Float                   m_softening;
    Float                   m_theta;
    bool                    m_parallel;
    IntegrateBodyFnT<Float> m_integrate_fn;

    // --- Tree Construction ---

    // NOTE: Compute bounding box for all bodies to determine Morton encoding
    // range.
    void compute_extent();

    // NOTE: Compute Morton codes for all bodies based on their positions.
    void compute_codes();

    // NOTE: Sort bodies by their Morton codes using radix sort.
    void sort_codes();

    // NOTE: Build the tree level-by-level from sorted Morton codes.
    // (1) Create leaf nodes at finest level needed.
    // (2) Propagate COM upward, creating internal nodes.
    void build_tree();

    // NOTE: Create leaf nodes from sorted body codes.
    // Bodies with identical prefixes at MAX_DEPTH are merged.
    void build_leaves();

    // NOTE: Build internal nodes for a specific level by grouping children from
    // level+1. Returns the range [start, end) of nodes created at this level.
    std::pair<USize, USize> build_level(USize level, USize children_start,
                                        USize children_end);

    // NOTE: Propagate center of mass from children to parent nodes.
    void propagate_com_up();

    // --- Force Calculation ---

    // NOTE: Compute acceleration on a body at given position using the
    // Barnes-Hut criterion. Self-interaction is avoided using position-based
    // distance check.
    [[nodiscard]] Vec2 compute_acceleration(Vec2  pos,
                                            USize skip_body_index) const;

    // NOTE: Recursive helper for tree traversal during force computation.
    void compute_acceleration_recursive(Vec2 pos, NodeIndex node_idx,
                                        Vec2& acc) const;

    // --- Utility ---

    // NOTE: Extract the quadrant index (0-3) from a Morton code at a given
    // level.
    [[nodiscard]] static U8 quadrant_at_level(MortonCode code, USize level);

    // NOTE: Extract the prefix of a Morton code at a given level (top bits
    // only).
    [[nodiscard]] static MortonCode prefix_at_level(MortonCode code,
                                                    USize      level);

    // NOTE: Compute the spatial size of a node at a given level.
    [[nodiscard]] Float size_at_level(USize level) const;
};

}  // namespace nbody::sim
