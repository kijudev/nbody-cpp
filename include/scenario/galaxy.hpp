// ==============================================================================
// galaxy.hpp
// Scenario: Simulates a globular galaxy using a Plummer distribution and
// circular orbital velocities. Uses the Barnes-Hut algorithm for efficient
// n-body simulation. Includes interactive slingshot body launching and UI.
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
// Galaxy Scenario
// ==============================================================================
// Simulates a galaxy with a Plummer distribution and circular velocities.
// Allows interactive launching of new bodies and camera control.
// ==============================================================================

class Galaxy : public ScenarioInterface {
   public:
    using Float = F64;
    using Vec2  = math::Vec2T<Float>;
    using Body  = sim::BodyT<F64>;

    Galaxy()  = default;
    ~Galaxy() = default;

    // Initialize the scenario with a new galaxy and simulation state.
    void init(const gfx::Window& window) override;
    // Advance the simulation and handle all per-frame logic.
    void step(const gfx::Window& window) override;

   private:
    // Handle keyboard/mouse input for simulation and camera.
    void handle_input();

    // Handle slingshot input for launching new bodies.
    void handle_slingshot_input(Float dt);

    // Advance the simulation state by dt.
    void update_sim(Float dt);

    // Update camera position and zoom.
    void update_camera(Float dt, const gfx::Window& window);

    // Draw all simulated bodies.
    void draw_sim();

    // Draw the slingshot UI overlay.
    void draw_slingshot();

    // Draw scenario UI (labels, controls, etc).
    void draw_ui(const gfx::Window& window);

    // Launch a new body using the slingshot state.
    void launch_slingshot_body();

    // --- Simulation state ---
    sim::BarnesHut<F64> m_sim{};
    Float               m_time_factor{sim::scale_au::TIME_YEAR};
    Float               m_scale_factor{15.0};
    Float               m_simulation_time{0.0};

    // --- Drawing state ---
    gfx::Camera<Float> m_camera{};
    gfx::Grid          m_grid{};

    // --- UI and scenario state ---
    bool  m_is_sim_running{true};
    bool  m_is_ui_visible{true};
    bool  m_is_sim_visible{true};
    bool  m_is_tracking_body{false};
    USize m_tracked_body_index{0};

    // --- Slingshot state for launching new bodies ---
    impl::SlingshotState<Float> m_slingshot_state{
        .base_mass      = sim::scale_au::MASS_EARTH,
        .radius_scale   = 0.5,
        .velocity_scale = 1.0,
    };
};

}  // namespace nbody::scenario
