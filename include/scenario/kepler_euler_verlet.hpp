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

class KeplerEulerVerlet : public ScenarioInterface {
   public:
    using Float     = F64;
    using Vec2      = math::Vec2T<Float>;
    using Body      = sim::BodyT<F64>;
    using DirectSim = sim::Direct<F64>;
    using KeplerSim = sim::Kepler<F64>;

    KeplerEulerVerlet();
    ~KeplerEulerVerlet() = default;

    void init(const gfx::Window& window) override;
    void step(const gfx::Window& window) override;

   private:
    void handle_input();
    void update_sim(Float dt);
    void update_camera(Float dt, const gfx::Window& window);
    void draw_sim();
    void draw_ui(const gfx::Window& window);

    // --- Simulations ---
    std::unique_ptr<KeplerSim>  m_kepler;
    std::unique_ptr<DirectSim>  m_direct_newton;
    std::unique_ptr<DirectSim>  m_direct_verlet;

    // --- Drawing ---
    gfx::Camera<Float> m_camera{};
    gfx::Grid          m_grid{};

    // --- State ---
    Float  m_simulation_time{0.0};
    Float  m_time_factor{sim::scale_au::TIME_DAY};
    bool   m_is_sim_running{true};
    bool   m_is_ui_visible{true};
};
}  // namespace nbody::scenario
