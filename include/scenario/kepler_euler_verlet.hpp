// ==============================================================================
// kepler_euler_verlet.hpp
// Scenario: Comparison between analytical Kepler solution and numerical
// integrators. Provides a scenario class that runs both Kepler and direct
// N-body simulations and visualizes their results for comparison.
// ==============================================================================

#pragma once

#include <memory>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"
#include "scenario/type.hpp"
#include "sim/const.hpp"
#include "sim/direct.hpp"
#include "sim/kepler.hpp"
#include "sim/type.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

// ==============================================================================
// KeplerEulerVerlet Scenario
// ==============================================================================
// Scenario for comparing analytical Kepler solution with direct N-body
// integration. Runs both simulations in parallel and visualizes their orbits
// and states.
class KeplerEulerVerlet : public ScenarioInterface {
   public:
    using Float     = F64;                 // Floating-point type for simulation
    using Vec2      = math::Vec2T<Float>;  // 2D vector type
    using Body      = sim::BodyT<F64>;     // Body type
    using DirectSim = sim::Direct<F64>;    // Direct N-body simulation
    using KeplerSim = sim::Kepler<F64>;    // Analytical Kepler simulation

    KeplerEulerVerlet();
    ~KeplerEulerVerlet() = default;

    // Initialize the scenario with window properties and initial conditions.
    void init(const gfx::Window& window) override;
    // Advance the simulation by one step and handle rendering.
    void step(const gfx::Window& window) override;

   private:
    // Handle user input (UI, controls, etc).
    void handle_input();

    // Update all simulations by dt.
    void update_sim(Float dt);

    // Update camera position and zoom.
    void update_camera(Float dt, const gfx::Window& window);

    // Draw the simulation state (bodies, orbits, etc).
    void draw_sim();

    // Draw the scenario UI (labels, controls, etc).
    void draw_ui(const gfx::Window& window);

    // --- Simulations ---
    std::unique_ptr<KeplerSim> m_kepler;         // Analytical Kepler simulation
    std::unique_ptr<DirectSim> m_direct_newton;  // Direct N-body (Euler)
    std::unique_ptr<DirectSim> m_direct_verlet;  // Direct N-body (Verlet)

    // --- Drawing ---
    gfx::Camera<Float> m_camera{};  // Camera for visualization
    gfx::Grid          m_grid{};    // UI grid layout

    // --- State ---
    Float m_simulation_time{0.0};                  // Current simulation time
    Float m_time_factor{sim::scale_au::TIME_DAY};  // Time scaling factor
    bool  m_is_sim_running{true};                  // Is the simulation running?
    bool  m_is_ui_visible{true};                   // Is the UI visible?
};
}  // namespace nbody::scenario
