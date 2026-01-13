#include <raygui.h>
#include <raylib.h>

#include <cstring>

#include "base/type.hpp"
#include "gfx/window.hpp"
#include "scenario/bhm_40k.hpp"
#include "scenario/draw.hpp"
#include "sim/const.hpp"
#include "sim/generator.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

void BHM40K::init(const gfx::Window& window) {
    sim::GenerateDistributionConfig<Float> generate_distribution_config{
        .n           = 10000,
        .min_mass    = sim::scale_au::MASS_HYGIEA,
        .max_mass    = sim::scale_au::MASS_SOL * 10,
        .radius      = sim::scale_au::DISTANCE_AU * 10.0,
        .position_fn = sim::generate_position_distribution_plummer_model<Float>,
        .mass_fn     = sim::generate_mass_distribution_salpeter_imf<Float>,
    };

    std::vector<Body> bodies =
        sim::generate_distribution(generate_distribution_config);

    sim::BarnesHutMorton<Float, U64>::Config sim_config{
        .bodies       = std::move(bodies),
        .g            = sim::scale_au::G,
        .softening    = sim::scale_au::SOFTENING,
        .parallel     = true,
        .radix        = true,
        .integrate_fn = sim::integrate_body_verlet<Float>,
    };

    m_sim = sim::BarnesHutMorton<Float, U64>(sim_config);

    m_camera = {
        .screen_width   = window.width,
        .screen_height  = window.height,
        .zoom           = 0.1,
        .movement_speed = sim::scale_au::DISTANCE_AU * 100.0,
        .scaling_speed  = 10.0,
    };

    m_grid = {
        .width  = window.width,
        .height = window.height,
        .cols   = 16,
        .rows   = 12,
    };
}

void BHM40K::step(const gfx::Window& window) {
    Float dt = static_cast<Float>(GetFrameTime());

    handle_input();
    update_sim(dt);
    update_camera(dt, window);
    draw_sim();
    draw_ui(window);
}

void BHM40K::handle_input() {
    if (IsKeyPressed(KEY_SPACE)) {
        m_is_sim_running = !m_is_sim_running;
    }

    if (IsKeyPressed(KEY_H)) {
        m_is_ui_visible = !m_is_ui_visible;
    }

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_T)) {
        m_is_tracking_body = false;
    }

    if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
        m_time_factor *= 2.0;
    }
    if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
        m_time_factor /= 2.0;
        if (m_time_factor < sim::scale_au::TIME_MINUTE) {
            m_time_factor = sim::scale_au::TIME_MINUTE;
        }
    }

    if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
        m_scale_factor *= 1.5;
    }
    if (IsKeyPressed(KEY_LEFT_BRACKET)) {
        m_scale_factor /= 1.5;

        if (m_scale_factor < 1.0) {
            m_scale_factor = 1.0;
        }
    }
}

void BHM40K::update_sim(Float dt) {
    if (!m_is_sim_running) {
        return;
    }

    Float sim_dt = dt * m_time_factor;
    m_sim.step(sim_dt);
    m_simulation_time += sim_dt;
}

void BHM40K::update_camera(Float dt, const gfx::Window& window) {
    m_camera.screen_width  = window.width;
    m_camera.screen_height = window.height;

    if (m_is_tracking_body) {
        m_camera.handle_controls_zoom(dt);
    } else {
        m_camera.handle_controls_movement(dt);
        m_camera.handle_controls_zoom(dt);
    }

    if (m_is_tracking_body && m_tracked_body_index < m_sim.bodies().size()) {
        m_camera.pos = m_sim.bodies()[m_tracked_body_index].pos;
    }
}

void BHM40K::draw_sim() {
    if (!m_is_sim_visible) {
        return;
    }

    impl::draw_bodies_monocolor(m_camera, m_scale_factor, m_sim.bodies(),
                                WHITE);
}

void BHM40K::draw_ui(const gfx::Window& window) {
    if (!m_is_ui_visible) {
        return;
    }

    m_grid.width  = window.width;
    m_grid.height = window.height;

    impl::draw_cross_center(m_camera, 20, 2, WHITE);
    impl::draw_ruler_au(m_camera, 32, 32, 0.5);
}
}  // namespace nbody::scenario
