#include "scenario/bhm_40k.hpp"

#include <raylib.h>

#include "base/type.hpp"
#include "gfx/draw.hpp"
#include "gfx/window.hpp"
#include "sim/const.hpp"
#include "sim/generator.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

void BHM40K::init(const gfx::Window& window) {
    sim::GenerateDistributionConfig<Float> generate_distribution_config{
        .n           = 40000,
        .min_mass    = sim::scale_au::MASS_HYGIEA,
        .max_mass    = sim::scale_au::MASS_SOL * 10,
        .radius      = sim::scale_au::DISTANCE_AU * 2000.0,
        .position_fn = sim::generate_position_distribution_plummer_model<Float>,
        .mass_fn     = sim::generate_mass_distribution_salpeter_imf<Float>,
    };

    std::vector<Body> bodies = sim::generate_distribution(generate_distribution_config);

    sim::BarnesHutMorton<Float, U64>::Config sim_config{
        .bodies       = std::move(bodies),
        .g            = sim::scale_au::G,
        .softening    = sim::scale_au::SOFTENING,
        .parallel     = true,
        .radix        = true,
        .integrate_fn = sim::integrate_body_verlet<Float>,
    };

    sim::BarnesHutMorton<Float, U64> sim(sim_config);

    m_camera = {
        .screen_width   = window.width,
        .screen_height  = window.height,
        .zoom           = 10.0,
        .movement_speed = sim::scale_au::DISTANCE_AU / sim::scale_au::TIME_MINUTE,
        .scaling_speed  = sim::scale_au::DISTANCE_AU,
    };
}

void BHM40K::step(const gfx::Window& window) {
    Float dt = static_cast<Float>(GetFrameTime());

    m_camera.screen_width  = window.width;
    m_camera.screen_height = window.height;

    m_camera.handle_controls(dt);

    if (!m_is_paused) {
        m_sim_bhm.step(dt * m_time_factor);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    gfx::draw_sim_bodies(m_camera, m_scale_factor, m_sim_bhm.bodies());

    EndDrawing();
}

void BHM40K::controls() {
    if (IsKeyPressed(KEY_SPACE)) {
        m_is_paused = !m_is_paused;
    }
}

void BHM40K::ui() { DrawText("BHM 40k", 10, 10, 20, WHITE); }
}  // namespace nbody::scenario
