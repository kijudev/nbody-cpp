#include "scenario/kepler_euler_verlet.hpp"

#include <raygui.h>
#include <raylib.h>

#include <format>
#include <string>
#include <vector>

#include "gfx/box.hpp"
#include "gfx/const.hpp"
#include "gfx/window.hpp"
#include "scenario/draw.hpp"
#include "scenario/impl.hpp"
#include "sim/const.hpp"
#include "sim/direct.hpp"
#include "sim/kepler.hpp"
#include "sim/type.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

KeplerEulerVerlet::KeplerEulerVerlet() = default;

void KeplerEulerVerlet::init(const gfx::Window& window) {
    // --- Initial conditions ---
    Float central_mass   = sim::scale_au::MASS_SOL;
    Float orbiter_mass   = sim::scale_au::MASS_EARTH;
    Float orbital_radius = 1.0;  // AU
    Float g              = sim::scale_au::G;

    // --- Kepler simulation (analytical) ---
    sim::Kepler<Float>::Config kepler_config{
        .body1                       = {.pos  = {0.0, 0.0},
                                        .vel  = {0.0, 0.0},
                                        .acc  = {0.0, 0.0},
                                        .mass = central_mass},
        .body2                       = {.pos  = {orbital_radius, 0.0},
                                        .vel  = {0.0, std::sqrt(g * central_mass / orbital_radius)},
                                        .acc  = {0.0, 0.0},
                                        .mass = orbiter_mass},
        .g                           = g,
        .epoch                       = 0.0,
        .compute_elements_from_state = true
    };
    m_kepler = std::make_unique<sim::Kepler<Float>>(kepler_config);

    std::vector<Body> bodies_newton{
        {.pos  = {0.0, 0.0},
         .vel  = {0.0, 0.0},
         .acc  = {0.0, 0.0},
         .mass = central_mass},
        {.pos  = {orbital_radius, 0.0},
         .vel  = {0.0, std::sqrt(g * central_mass / orbital_radius)},
         .acc  = {0.0, 0.0},
         .mass = orbiter_mass}
    };
    sim::Direct<Float>::Config direct_newton_config{
        .bodies       = bodies_newton,
        .g            = g,
        .softening    = sim::scale_au::SOFTENING,
        .parallel     = false,
        .integrate_fn = sim::integrate_body_euler<Float>};
    m_direct_newton = std::make_unique<sim::Direct<Float>>(direct_newton_config);

    std::vector<Body> bodies_verlet{
        {.pos  = {0.0, 0.0},
         .vel  = {0.0, 0.0},
         .acc  = {0.0, 0.0},
         .mass = central_mass},
        {.pos  = {orbital_radius, 0.0},
         .vel  = {0.0, std::sqrt(g * central_mass / orbital_radius)},
         .acc  = {0.0, 0.0},
         .mass = orbiter_mass}
    };
    sim::Direct<Float>::Config direct_verlet_config{
        .bodies       = bodies_verlet,
        .g            = g,
        .softening    = sim::scale_au::SOFTENING,
        .parallel     = false,
        .integrate_fn = sim::integrate_body_verlet<Float>};
    m_direct_verlet = std::make_unique<sim::Direct<Float>>(direct_verlet_config);

    m_camera = {
        .screen_width   = window.width,
        .screen_height  = window.height,
        .pos            = {0.0, 0.0},
        .zoom           = 200.0,
        .movement_speed = 1.0,
        .scaling_speed  = 10.0,
    };

    m_grid = {
        .width  = window.width,
        .height = window.height,
        .cols   = 16,
        .rows   = 12,
    };

    m_simulation_time = 0.0;
    m_is_sim_running  = true;
    m_is_ui_visible   = true;
    m_time_factor     = sim::scale_au::TIME_DAY;
}

void KeplerEulerVerlet::step(const gfx::Window& window) {
    Float dt = static_cast<Float>(GetFrameTime());

    handle_input();
    update_sim(dt);
    update_camera(dt, window);
    draw_sim();
    draw_ui(window);
}

void KeplerEulerVerlet::handle_input() {
    if (IsKeyPressed(KEY_SPACE)) {
        m_is_sim_running = !m_is_sim_running;
    }
    if (IsKeyPressed(KEY_H)) {
        m_is_ui_visible = !m_is_ui_visible;
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
        m_camera.zoom *= 1.2;
    }
    if (IsKeyPressed(KEY_LEFT_BRACKET)) {
        m_camera.zoom /= 1.2;
        if (m_camera.zoom < 1.0) m_camera.zoom = 1.0;
    }
}

void KeplerEulerVerlet::update_sim(Float dt) {
    if (!m_is_sim_running) return;

    Float sim_dt = dt * m_time_factor;
    m_kepler->step(sim_dt);
    m_direct_newton->step(sim_dt);
    m_direct_verlet->step(sim_dt);
    m_simulation_time += sim_dt;
}

void KeplerEulerVerlet::update_camera(Float dt, const gfx::Window& window) {
    m_camera.screen_width  = window.width;
    m_camera.screen_height = window.height;
    m_camera.handle_controls_movement(dt);
    m_camera.handle_controls_zoom(dt);
}

void KeplerEulerVerlet::draw_sim() {
    const auto& bodies_kepler = m_kepler->bodies();
    const auto& bodies_newton = m_direct_newton->bodies();
    const auto& bodies_verlet = m_direct_verlet->bodies();

    {
        const auto&              body = bodies_kepler[0];
        const math::Vec2T<Float> center =
            m_camera.world_to_screen_vec(body.pos);
        const math::Vec2T<Float> edge = m_camera.world_to_screen_vec(
            math::Vec2T<Float>{body.pos.x + std::cbrt(body.mass), body.pos.y});
        const Float radius = center.distance(edge) * 1.5;
        DrawCircleV(center.as_raylib_vector(), radius, GRAY);
    }

    {
        const auto&              body = bodies_kepler[1];
        const math::Vec2T<Float> center =
            m_camera.world_to_screen_vec(body.pos);
        DrawCircleV(center.as_raylib_vector(), 8.0f, gfx::YELLOW_WARM);

        const auto&              body_n = bodies_newton[1];
        const math::Vec2T<Float> center_n =
            m_camera.world_to_screen_vec(body_n.pos);
        DrawCircleV(center_n.as_raylib_vector(), 8.0f, gfx::RED_WARM);

        const auto&              body_v = bodies_verlet[1];
        const math::Vec2T<Float> center_v =
            m_camera.world_to_screen_vec(body_v.pos);
        DrawCircleV(center_v.as_raylib_vector(), 8.0f, gfx::BLUE_WARM);
    }
}

void KeplerEulerVerlet::draw_ui(const gfx::Window& window) {
    if (!m_is_ui_visible) return;

    m_grid.width  = window.width;
    m_grid.height = window.height;

    gfx::Box info_box = m_grid.span(0, 3, 0, 3)
                            .with_padding_left(gfx::S)
                            .with_padding_top(gfx::L)
                            .with_draw_background(BLACK);

    GuiGroupBox(info_box.rectangle(), "Simulation Comparison Info");

    std::vector<std::pair<std::string, std::string>> info{
        {"Integrator", "Kepler (Yellow), Newton/Euler (Red), Verlet (Blue)"},
        {"Sim Time", std::format("{:.2f} days", m_simulation_time)},
        {"Time Factor",
         impl::format_time(m_time_factor, sim::scale_au::TIME_YEAR,
         sim::scale_au::TIME_DAY, sim::scale_au::TIME_HOUR,
         sim::scale_au::TIME_MINUTE)},
        {"FPS", std::to_string(GetFPS())},
        {"[Space]", "Pause/Resume"},
        {"[H]", "Toggle UI"},
        {"[Arrows/WASD]", "Move Camera"},
        {"[Mouse Wheel]", "Zoom"},
        {"[= / +]", "Increase Time Factor"},
        {"[-]", "Decrease Time Factor"},
        {"[[ / ]]", "Zoom In/Out"}
    };

    impl::draw_label_pairs(info_box.with_padding_left(gfx::S).with_padding_top(
                               gfx::L + gfx::M * 2),
                           info, gfx::M, gfx::XS, 0, gfx::YELLOW_PALE,
                           gfx::YELLOW_WARM);

    gfx::Box legend_box = m_grid.span(0, 2, 4, 4)
                              .with_padding_left(gfx::S)
                              .with_padding_top(gfx::S)
                              .with_draw_background(BLACK);

    DrawText("Legend:", legend_box.x, legend_box.y, gfx::M, RAYWHITE);
    DrawCircle(legend_box.x + 10, legend_box.y + 30, 8, gfx::YELLOW_WARM);
    DrawText("Kepler (Analytical)", legend_box.x + 30, legend_box.y + 22,
             gfx::S, RAYWHITE);
    DrawCircle(legend_box.x + 10, legend_box.y + 55, 8, gfx::RED_WARM);
    DrawText("Direct Newton/Euler", legend_box.x + 30, legend_box.y + 47,
             gfx::S, RAYWHITE);
    DrawCircle(legend_box.x + 10, legend_box.y + 80, 8, gfx::BLUE_WARM);
    DrawText("Direct Verlet", legend_box.x + 30, legend_box.y + 72, gfx::S,
             RAYWHITE);
}

}  // namespace nbody::scenario
