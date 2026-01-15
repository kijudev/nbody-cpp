// ==============================================================================
// direct.hpp
// Direct implementation of the n-body problem (O(N^2) force calculation).
// Computes all pairwise interactions between bodies using Newtonian gravity.
// Supports parallelization and multiple integration schemes.
// ==============================================================================
#pragma once

#include <span>
#include <vector>

#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class Direct : public SimInterface<Float> {
   public:
    // 2D vector and body type aliases for convenience.
    using Vec2 = math::Vec2T<Float>;
    using Body = BodyT<Float>;

    // Configuration for the simulation.
    struct Config {
        std::vector<Body> bodies{};                  // Initial bodies
        Float             g{sim::scale_toy::G};      // Gravitational constant
        Float softening{sim::scale_toy::SOFTENING};  // Softening parameter
        bool  parallel{false};           // Enable parallel computation
        bool  use_proper_verlet{false};  // Use proper Velocity Verlet
        IntegrateBodyFnT<Float> integrate_fn{
            integrate_body_euler<Float>};  // Integration scheme
    };

    // Construct a Direct simulation with the given config.
    Direct(const Config& config);

    // Advance the simulation by one timestep (dt).
    void step(Float dt) override;

    // Insert a new body into the simulation.
    void insert_body(Body&& body) override;

    // Compute initial accelerations for all bodies (call before first step).
    void compute_initial_accelerations();

    // Return a span of all bodies in the simulation (read-only).
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies()
        const override;

   private:
    // Compute accelerations for all bodies (internal).
    void impl_compute_acc();

    // Parallel version of acceleration computation.
    void impl_compute_acc_par();

    // Sequential version of acceleration computation.
    void impl_compute_acc_seq();

    std::vector<Body> m_bodies;  // All bodies in the simulation
    std::vector<Vec2>
          m_old_accelerations;  // Previous accelerations (for Verlet)
    Float m_g;                  // Gravitational constant
    Float m_softening;          // Softening parameter
    bool  m_parallel;           // Parallel computation enabled
    bool  m_use_proper_verlet;  // Use proper Velocity Verlet
    IntegrateBodyFnT<Float> m_integrate_fn;  // Integration scheme
};
}  // namespace nbody::sim
