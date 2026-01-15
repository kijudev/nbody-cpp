// ==============================================================================
// barnes_hut_morton.hpp
// Barnes-Hut implementation using Morton codes and radix sort for fast tree
// construction. Efficient for large n-body systems.
// ==============================================================================
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

// ==============================================================================
// Barnes-Hut Morton Simulation
// ==============================================================================
// Implements the Barnes-Hut algorithm using Morton codes (Z-order curve) for
// spatial partitioning and radix sort for efficient tree construction.
// Suitable for very large n-body systems.
// WARNING: Bodies are reordered in memory during simulation steps for cache
// coherence.

template <FloatT Float, math::MortonCodeT MortonCode = U64>
class BarnesHutMorton : public SimInterface<Float> {
   public:
    // Type aliases for convenience.
    using Vec2 = math::Vec2T<Float>;  // 2D vector type
    using Body = BodyT<Float>;        // Body type

    // Morton code constants for tree depth and bit layout.
    static constexpr USize MORTON_BITS  = sizeof(MortonCode) * 8;
    static constexpr USize BITS_PER_DIM = MORTON_BITS / 2;
    static constexpr USize MAX_DEPTH    = BITS_PER_DIM;

    // ==============================================================================
    // Node Structure
    // ==============================================================================
    // Represents a node in the Morton-based quad tree (internal or leaf).
    struct Node {
        MortonCode prefix;          // Morton code prefix for this node
        U8         level;           // Tree level (depth)
        Vec2       center_of_mass;  // Center of mass of all bodies in node
        Float      total_mass;      // Total mass in node
        Float      size;            // Side length of node region
        USize      first_idx;       // Index of first body in Morton order
        USize      last_idx;        // Index of last body in Morton order
        USize      children[4];  // Indices of child nodes (NODE_EMPTY if none)

        static constexpr USize NODE_EMPTY = std::numeric_limits<USize>::max();

        [[nodiscard]] bool is_leaf() const {
            return children[0] == NODE_EMPTY && children[1] == NODE_EMPTY &&
                   children[2] == NODE_EMPTY && children[3] == NODE_EMPTY;
        }

        [[nodiscard]] USize body_count() const { return last_idx - first_idx; }
    };

    // ==============================================================================
    // MortonBody Structure
    // ==============================================================================
    // Associates a body index with its Morton code for sorting.
    struct MortonBody {
        MortonCode morton;    // Morton code for spatial ordering
        USize      body_idx;  // Index of the body in m_bodies

        bool operator<(const MortonBody& other) const {
            return morton < other.morton;
        }
    };

    // ==============================================================================
    // Configuration Struct
    // ==============================================================================
    // Specifies initial bodies, simulation parameters, and integration scheme.
    struct Config {
        std::vector<Body> bodies{};                  // Initial bodies
        Float             g{sim::scale_toy::G};      // Gravitational constant
        Float softening{sim::scale_toy::SOFTENING};  // Softening parameter
        Float theta{0.5};                            // Barnes-Hut opening angle
        bool  parallel{false};           // Enable parallel computation
        bool  radix{false};              // Use radix sort for Morton codes
        bool  use_proper_verlet{false};  // Use proper Velocity Verlet
        IntegrateBodyFnT<Float> integrate_fn{
            integrate_body_euler<Float>};  // Integration scheme
    };

    // ==============================================================================
    // Public Interface
    // ==============================================================================
    // Construct a Barnes-Hut Morton simulation with the given config.
    explicit BarnesHutMorton(const Config& config = Config{});

    // Advance the simulation by one timestep (dt).
    void step(Float dt) override;

    // Insert a new body into the simulation.
    void insert_body(Body&& body) override;

    // WARNING: Body order may change during simulation steps due to Morton
    // sorting.
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies()
        const override;

    // Return a reference to all quad tree nodes.
    [[nodiscard]] const std::vector<Node>& nodes() const { return m_nodes; }

    // Return a reference to all Morton bodies (Morton code + index).
    [[nodiscard]] const std::vector<MortonBody>& morton_bodies() const {
        return m_morton_bodies;
    }

   private:
    // ==============================================================================
    // Tree Construction Methods
    // ==============================================================================

    // Build the quad tree from current body positions.
    void build_tree();

    // Compute Morton codes for all bodies.
    void compute_morton_codes();

    // Sort bodies by Morton code (radix or std::sort).
    void sort_by_morton();

    // Recursively build tree nodes from Morton-ordered bodies.
    void build_nodes_recursive(USize first, USize last, U8 level,
                               MortonCode prefix, Float node_size,
                               Vec2 node_center);

    // Compute center of mass and mass for a node.
    void compute_node_properties(USize node_idx);

    // ==============================================================================
    // Force Calculation Methods
    // ==============================================================================
    // Compute acceleration for a body by traversing the tree.
    Vec2 compute_acceleration(USize body_idx) const;
    // Compute acceleration from a specific node.
    Vec2 compute_acceleration_from_node(USize node_idx, Vec2 pos) const;

    // ==============================================================================
    // Utility Methods
    // ==============================================================================
    // Get the center of a node from its Morton prefix and level.
    [[nodiscard]] Vec2 get_node_center_from_prefix(MortonCode prefix,
                                                   U8         level) const;
    // Get the size of a node at a given level.
    [[nodiscard]] Float get_node_size(U8 level) const;

    // Get the quadrant index from Morton code and level.
    [[nodiscard]] U8 get_quadrant(MortonCode code, U8 level) const;

    // ==============================================================================
    // Member Variables
    // ==============================================================================
    std::vector<Body>
        m_bodies;  // All bodies in the simulation (may be reordered)
    std::vector<MortonBody>
                      m_morton_bodies;  // Morton code + index for each body
    std::vector<Node> m_nodes;          // Quad tree nodes
    std::vector<Vec2>
        m_old_accelerations;  // Previous accelerations (for Verlet)

    Float m_g;                  // Gravitational constant
    Float m_softening;          // Softening parameter
    Float m_theta;              // Barnes-Hut opening angle
    bool  m_parallel;           // Parallel computation enabled
    bool  m_radix;              // Use radix sort for Morton codes
    bool  m_use_proper_verlet;  // Use proper Velocity Verlet

    Float m_bounds_min;  // Minimum bounds for Morton encoding
    Float m_bounds_max;  // Maximum bounds for Morton encoding

    IntegrateBodyFnT<Float> m_integrate_fn;  // Integration scheme
};
}  // namespace nbody::sim
