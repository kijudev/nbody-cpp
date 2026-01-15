// ==============================================================================
// barnes_hut_morton_plummer.hpp
// Scenario: Barnes-Hut simulation using Morton codes with a Plummer
// distribution. Handles initialization, simulation stepping, input, drawing,
// and slingshot UI.
// ==============================================================================

#pragma once

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"
#include "math/vec.hpp"
#include "scenario/impl.hpp"
#include "scenario/type.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/const.hpp"
#include "sim/type.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

// ==============================================================================
// BarnesHutMortonPlummer Scenario
// ==============================================================================
// Implements a scenario with a large number of bodies using a Plummer
// distribution and a Barnes-Hut tree built with Morton codes for efficient
// spatial partitioning. Includes interactive slingshot launching and UI
// controls.
class BarnesHutMortonPlummer : public ScenarioInterface {
   public:
    using Float = F64;
    using Vec2  = math::Vec2T<Float>;
    using Body  = sim::BodyT<F64>;
    using Sim   = sim::BarnesHutMorton<F64, U64>;

    BarnesHutMortonPlummer()  = default;
    ~BarnesHutMortonPlummer() = default;

    // Initialize the scenario (bodies, simulation, camera, etc.)
    void init(const gfx::Window& window) override;
    // Advance the simulation and handle all per-frame logic.
    void step(const gfx::Window& window) override;

   private:
    // Handle keyboard/mouse input for simulation controls.
    void handle_input();

    // Handle slingshot input for launching new bodies.
    void handle_slingshot_input(Float dt);

    // Advance the simulation state by dt.
    void update_sim(Float dt);

    // Update camera position/zoom based on input and window size.
    void update_camera(Float dt, const gfx::Window& window);

    // Draw all simulation bodies and overlays.
    void draw_sim();

    // Draw the slingshot UI overlay.
    void draw_slingshot();

    // Draw scenario UI (stats, controls, etc.).
    void draw_ui(const gfx::Window& window);

    // Launch a new body using the slingshot state.
    void launch_slingshot_body();

    // --- Simulation state ---
    Sim   m_sim{};  // The Barnes-Hut Morton simulation
    Float m_time_factor{sim::scale_au::TIME_YEAR};  // Time scaling factor
    Float m_scale_factor{50.0};                     // Visual scale factor
    Float m_simulation_time{0.0};  // Total simulation time elapsed

    // --- Drawing state ---
    gfx::Camera<Float> m_camera{};  // Camera for world <-> screen
    gfx::Grid          m_grid{};    // UI grid layout

    // --- UI and scenario state ---
    bool  m_is_sim_running{true};     // Is the simulation running?
    bool  m_is_ui_visible{true};      // Is the UI visible?
    bool  m_is_sim_visible{true};     // Are simulation bodies visible?
    bool  m_is_tracking_body{false};  // Is camera tracking a body?
    USize m_tracked_body_index{0};    // Index of tracked body

    // --- Slingshot state for launching new bodies interactively ---
    impl::SlingshotState<Float> m_slingshot_state{
        .base_mass      = sim::scale_au::MASS_EARTH,
        .radius_scale   = 0.5,
        .velocity_scale = 1.0,
    };
};
}  // namespace nbody::scenario
