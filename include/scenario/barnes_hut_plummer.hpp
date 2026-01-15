// ==============================================================================
// barnes_hut_plummer.hpp
// Scenario: Barnes-Hut simulation with Plummer distribution (many bodies).
// Provides interactive simulation with slingshot launching and UI controls.
// ==============================================================================

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
// BarnesHutPlummer Scenario
// ==============================================================================
// Implements a scenario using the Barnes-Hut algorithm with a Plummer
// distribution of bodies. Supports interactive slingshot launching, UI toggles,
// and camera controls.
//
class BarnesHutPlummer : public ScenarioInterface {
   public:
    using Float = F64;                 // Floating-point type for simulation
    using Vec2  = math::Vec2T<Float>;  // 2D vector type
    using Body  = sim::BodyT<F64>;     // Body type

    BarnesHutPlummer()  = default;
    ~BarnesHutPlummer() = default;

    // Initialize the scenario (generate bodies, set up simulation).
    void init(const gfx::Window& window) override;
    // Advance the simulation and handle all per-frame logic.
    void step(const gfx::Window& window) override;

   private:
    // Handle keyboard/mouse input for simulation controls.
    void handle_input();

    // Handle slingshot input (drag to launch new body).
    void handle_slingshot_input(Float dt);

    // Advance the simulation state by dt.
    void update_sim(Float dt);

    // Update camera position/zoom based on input.
    void update_camera(Float dt, const gfx::Window& window);

    // Draw all simulation bodies and overlays.
    void draw_sim();

    // Draw the slingshot UI overlay.
    void draw_slingshot();

    // Draw scenario UI (stats, controls, etc).
    void draw_ui(const gfx::Window& window);

    // Launch a new body using the slingshot state.
    void launch_slingshot_body();

    // --- Simulation state ---
    sim::BarnesHut<F64> m_sim{};  // Barnes-Hut simulation instance
    Float m_time_factor{sim::scale_au::TIME_YEAR};  // Time scaling
    Float m_scale_factor{50.0};                     // Visual scale
    Float m_simulation_time{0.0};                   // Elapsed sim time

    // --- Drawing state ---
    gfx::Camera<Float> m_camera{};  // Camera for world <-> screen
    gfx::Grid          m_grid{};    // UI grid layout

    // --- UI/interaction state ---
    bool  m_is_sim_running{true};     // Is simulation running?
    bool  m_is_ui_visible{true};      // Is UI visible?
    bool  m_is_sim_visible{true};     // Are bodies visible?
    bool  m_is_tracking_body{false};  // Is camera tracking a body?
    USize m_tracked_body_index{0};    // Index of tracked body

    // --- Slingshot state ---
    impl::SlingshotState<Float> m_slingshot_state{
        .base_mass      = sim::scale_au::MASS_EARTH,
        .radius_scale   = 0.5,
        .velocity_scale = 1.0,
    };
};
}  // namespace nbody::scenario
