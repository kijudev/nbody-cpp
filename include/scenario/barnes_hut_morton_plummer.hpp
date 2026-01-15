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

class BarnesHutMortonPlummer : public ScenarioInterface {
   public:
    using Float = F64;
    using Vec2  = math::Vec2T<Float>;
    using Body  = sim::BodyT<F64>;
    using Sim   = sim::BarnesHutMorton<F64, U64>;

    BarnesHutMortonPlummer()  = default;
    ~BarnesHutMortonPlummer() = default;

    void init(const gfx::Window& window) override;
    void step(const gfx::Window& window) override;

   private:
    void handle_input();
    void handle_slingshot_input(Float dt);
    void update_sim(Float dt);
    void update_camera(Float dt, const gfx::Window& window);
    void draw_sim();
    void draw_slingshot();
    void draw_ui(const gfx::Window& window);
    void launch_slingshot_body();

    // --- Simulation ---
    Sim   m_sim{};
    Float m_time_factor{sim::scale_au::TIME_YEAR};
    Float m_scale_factor{50.0};
    Float m_simulation_time{0.0};

    // --- Drawing ---
    gfx::Camera<Float> m_camera{};
    gfx::Grid          m_grid{};

    // --- State ---
    bool  m_is_sim_running{true};
    bool  m_is_ui_visible{true};
    bool  m_is_sim_visible{true};
    bool  m_is_tracking_body{false};
    USize m_tracked_body_index{0};

    // --- Slingshot State ---
    impl::SlingshotState<Float> m_slingshot_state{
        .base_mass      = sim::scale_au::MASS_EARTH,
        .radius_scale   = 0.5,
        .velocity_scale = 1.0,
    };
};
}  // namespace nbody::scenario
