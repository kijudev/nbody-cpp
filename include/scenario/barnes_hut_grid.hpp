// ==============================================================================
// barnes_hut_grid.hpp
// Scenario: Barnes-Hut simulation with grid visualization and interactive UI.
// Implements a scenario with a Barnes-Hut simulation, grid overlay, slingshot
// body launching, and camera controls.
// ==============================================================================

#pragma once

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"
#include "math/vec.hpp"
#include "scenario/impl.hpp"
#include "scenario/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/const.hpp"
#include "sim/type.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

// ==============================================================================
// BarnesHutGrid Scenario
// ==============================================================================
// Implements a scenario with a Barnes-Hut simulation, grid overlay, interactive
// slingshot body launching, and camera controls. Useful for visualizing the
// Barnes-Hut algorithm and spatial partitioning.

class BarnesHutGrid : public ScenarioInterface {
   public:
    using Float = F64;                 // Floating-point type for simulation
    using Vec2  = math::Vec2T<Float>;  // 2D vector type
    using Body  = sim::BodyT<F64>;     // Body type
    using Quad = sim::BarnesHut<F64>::Quad;  // Quad type for grid visualization

    BarnesHutGrid()  = default;
    ~BarnesHutGrid() = default;

    // Initialize the scenario with the given window properties.
    void init(const gfx::Window& window) override;
    // Advance the simulation and handle UI for one frame.
    void step(const gfx::Window& window) override;

   private:
    // Handle keyboard and mouse input for simulation and UI.
    void handle_input();

    // Handle slingshot input for launching new bodies.
    void handle_slingshot_input(Float dt);

    // Advance the simulation state by dt.
    void update_sim(Float dt);

    // Update camera position and zoom based on input.
    void update_camera(Float dt, const gfx::Window& window);

    // Draw all simulation bodies and related visuals.
    void draw_sim();

    // Draw the grid overlay for spatial visualization.
    void draw_grid_visualization();

    // Draw the slingshot UI overlay.
    void draw_slingshot();

    // Draw scenario-specific UI (controls, stats, etc).
    void draw_ui(const gfx::Window& window);

    // Launch a new body using the slingshot state.
    void launch_slingshot_body();

    // --- Simulation State ---
    sim::BarnesHut<F64> m_sim{};  // Barnes-Hut simulation instance
    Float m_time_factor{sim::scale_au::TIME_YEAR};  // Time scaling factor
    Float m_scale_factor{50.0};                     // Visual scale factor
    Float m_simulation_time{0.0};                   // Elapsed simulation time

    // --- Drawing State ---
    gfx::Camera<Float> m_camera{};  // Camera for world <-> screen transforms
    gfx::Grid          m_grid{};    // Grid overlay for visualization

    // --- UI/Control State ---
    bool  m_is_sim_running{true};     // Is the simulation running?
    bool  m_is_ui_visible{true};      // Is the UI visible?
    bool  m_is_sim_visible{true};     // Are simulation bodies visible?
    bool  m_is_grid_visible{true};    // Is the grid overlay visible?
    bool  m_is_tracking_body{false};  // Is camera tracking a body?
    USize m_tracked_body_index{0};    // Index of tracked body

    // --- Slingshot State ---
    impl::SlingshotState<Float> m_slingshot_state{
        .base_mass      = sim::scale_au::MASS_EARTH,
        .radius_scale   = 0.5,
        .velocity_scale = 1.0,
    };
};

}  // namespace nbody::scenario
