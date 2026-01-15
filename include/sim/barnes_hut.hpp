// ==============================================================================
// barnes_hut.hpp
// Standard Barnes-Hut implementation using a linear quad tree.
// Efficiently approximates n-body interactions by grouping distant bodies.
// Supports parallelization and multiple integration schemes.
// ==============================================================================

#pragma once

#include <array>
#include <span>

#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// ==============================================================================
// Barnes-Hut Simulation
// ==============================================================================
// Implements the Barnes-Hut algorithm using a linear quad tree for spatial
// partitioning. Suitable for large n-body systems where O(N^2) direct
// computation is too expensive.

template <FloatT Float>
class BarnesHut : public SimInterface<Float> {
   public:
    // Type aliases for convenience.
    using Vec2      = math::Vec2T<Float>;  // 2D vector type
    using Body      = BodyT<Float>;        // Body type
    using QuadIndex = U8;                  // Index for quad subdivision
    using NodeIndex = USize;               // Index for quad tree nodes

    // Quad tree node index representing an empty node.
    static constexpr NodeIndex NODE_INDEX_EMPTY =
        std::numeric_limits<NodeIndex>::max();

    // ==============================================================================
    // Quad Structure
    // ==============================================================================

    // Represents a square region in space for quad tree partitioning.
    struct Quad {
        Vec2  center;  // Center of the quad
        Float size;    // Side length of the quad

        // Create a quad that contains all bodies in the span.
        static Quad make_containing_bodies(
            std::span<Body, std::dynamic_extent> bodies);

        // Returns the sub-quad for the given index (0-3).
        Quad into_quad(QuadIndex quad_index) const;

        // Returns all four sub-quads.
        std::array<Quad, 4> into_quads() const;

        // Returns the sub-quad containing the given position.
        Quad quad_from_pos(Vec2 pos) const;

        // Returns the index of the sub-quad containing the given position.
        QuadIndex quad_index_from_pos(Vec2 pos) const;
    };

    // ==============================================================================
    // Node Structure
    // ==============================================================================
    // Represents a node in the quad tree (internal or leaf).
    struct Node {
        NodeIndex children_index;  // Index of first child node (if internal)
        NodeIndex next_index;      // Index of next node in traversal
        Quad      quad;            // Spatial region of this node
        Vec2      pos;             // Center of mass position (if leaf)
        Float     mass;            // Total mass in this node

        // Create an empty node with the given next index and quad.
        static Node make_empty(NodeIndex next_index, Quad quad);

        // Returns true if node is empty (no mass).
        bool is_empty() const;
        // Returns true if node is internal (has children).
        bool is_internal() const;
        // Returns true if node is a leaf (no children, has mass).
        bool is_leaf() const;
    };

    // ==============================================================================
    // QuadTree Structure
    // ==============================================================================
    // Linear quad tree for Barnes-Hut spatial partitioning.
    struct QuadTree {
        std::vector<Node>      nodes;           // All nodes in the tree
        std::vector<NodeIndex> parent_indices;  // Indices of parent nodes

        // Clear the tree and initialize with the root quad.
        void clear(Quad quad);
        // Subdivide a node into four children.
        NodeIndex subdivide(NodeIndex node_index);
        // Insert a body at the given position and mass.
        void insert(Vec2 pos, Float mass);
        // Propagate center of mass and mass up the tree.
        void propagate_up_pos_mass();
        // Compute acceleration at a position using the tree.
        Vec2 propagate_down_acc(Vec2 pos, Float g, Float softening,
                                Float theta) const;
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
        bool  use_proper_verlet{false};  // Use proper Velocity Verlet
        IntegrateBodyFnT<Float> integrate_fn{
            integrate_body_euler<Float>};  // Integration scheme
    };

    // ==============================================================================
    // Public Interface
    // ==============================================================================
    // Construct a Barnes-Hut simulation with the given config.
    BarnesHut(const Config& config = {});

    // Advance the simulation by one timestep (dt).
    void step(Float dt) override;

    // Insert a new body into the simulation.
    void insert_body(Body&& body) override;

    // Return a span of all bodies in the simulation (read-only).
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies()
        const override;

    // Return a vector of all quads in the quad tree (for
    // visualization/debugging).
    [[nodiscard]] std::vector<Quad> quads() const;

   private:
    // --- Member Variables ---
    std::vector<Body> m_bodies;  // All bodies in the simulation
    std::vector<Vec2>
             m_old_accelerations;  // Previous accelerations (for Verlet)
    QuadTree m_quad_tree;          // Linear quad tree for spatial partitioning
    Float    m_g;                  // Gravitational constant
    Float    m_softening;          // Softening parameter
    Float    m_theta;              // Barnes-Hut opening angle
    bool     m_parallel;           // Parallel computation enabled
    bool     m_use_proper_verlet;  // Use proper Velocity Verlet
    IntegrateBodyFnT<Float> m_integrate_fn;  // Integration scheme
};
}  // namespace nbody::sim
