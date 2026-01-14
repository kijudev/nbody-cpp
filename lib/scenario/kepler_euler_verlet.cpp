#include "scenario/kepler_euler_verlet.hpp"

#include <raygui.h>
#include <raylib.h>

#include <format>
#include <memory>
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

    std::vector<Body> bodies_euler{
        {.pos  = {0.0, 0.0},
         .vel  = {0.0, 0.0},
         .acc  = {0.0, 0.0},
         .mass = central_mass},
        {.pos  = {orbital_radius, 0.0},
         .vel  = {0.0, std::sqrt(g * central_mass / orbital_radius)},
         .acc  = {0.0, 0.0},
         .mass = orbiter_mass}
    };

    sim::Direct<Float>::Config direct_euler_config{
        .bodies            = bodies_euler,
        .g                 = g,
        .softening         = sim::scale_au::SOFTENING,
        .parallel          = false,
        .use_proper_verlet = false,
        .integrate_fn      = sim::integrate_body_euler<Float>};
    m_direct_newton = std::make_unique<sim::Direct<Float>>(direct_euler_config);

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
        .bodies            = bodies_verlet,
        .g                 = g,
        .softening         = sim::scale_au::SOFTENING,
        .parallel          = false,
        .use_proper_verlet = true,  // Enable proper Velocity Verlet
        .integrate_fn      = sim::integrate_body_verlet<Float>};
    m_direct_verlet =
        std::make_unique<sim::Direct<Float>>(direct_verlet_config);

    // Initialize accelerations for proper Verlet by computing initial forces
    // This ensures the first timestep has valid old accelerations
    m_direct_verlet->compute_initial_accelerations();

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

    // Draw orbital path (white circle for reference)
    {
        const auto&              body = bodies_kepler[0];
        const math::Vec2T<Float> center =
            m_camera.world_to_screen_vec(body.pos);

        // Calculate orbital radius from Kepler's orbital elements
        Float orbital_radius = m_kepler->orbital_elements().semi_major_axis;
        math::Vec2T<Float> edge_pos = {body.pos.x + orbital_radius, body.pos.y};
        gfx::Point         edge_screen = m_camera.world_to_screen(edge_pos);

        Float screen_radius = std::abs(edge_screen.x - center.x);
        DrawCircleLines(center.x, center.y, screen_radius,
                        ColorAlpha(WHITE, 0.3f));
    }

    // Draw central body (Sun) - fixed size for visibility
    {
        const auto&              body = bodies_kepler[0];
        const math::Vec2T<Float> center =
            m_camera.world_to_screen_vec(body.pos);
        DrawCircleV(center.as_raylib_vector(), 15.0f, YELLOW);
    }

    // Draw orbiting bodies (Earth) - all three simulations
    {
        const auto&              body = bodies_kepler[1];
        const math::Vec2T<Float> center =
            m_camera.world_to_screen_vec(body.pos);
        DrawCircleV(center.as_raylib_vector(), 6.0f, gfx::YELLOW_WARM);

        const auto&              body_n = bodies_newton[1];
        const math::Vec2T<Float> center_n =
            m_camera.world_to_screen_vec(body_n.pos);
        DrawCircleV(center_n.as_raylib_vector(), 6.0f, gfx::RED_WARM);

        const auto&              body_v = bodies_verlet[1];
        const math::Vec2T<Float> center_v =
            m_camera.world_to_screen_vec(body_v.pos);
        DrawCircleV(center_v.as_raylib_vector(), 6.0f, gfx::BLUE_WARM);
    }
}

void KeplerEulerVerlet::draw_ui(const gfx::Window& window) {
    if (!m_is_ui_visible) return;

    m_grid.width  = window.width;
    m_grid.height = window.height;

    // Info panel
    gfx::Box info_box = m_grid.span(0, 3, 0, 5)
                            .with_padding_left(gfx::S)
                            .with_padding_top(gfx::L)
                            .with_draw_background(BLACK);

    GuiGroupBox(info_box.rectangle(), "Simulation Info");

    std::vector<std::pair<std::string, std::string>> info{
        {"Sim Time", std::format("{:.2f} days",
         m_simulation_time / sim::scale_au::TIME_DAY)},
        {"Time Factor",
         impl::format_time(m_time_factor, sim::scale_au::TIME_YEAR,
         sim::scale_au::TIME_DAY, sim::scale_au::TIME_HOUR,
         sim::scale_au::TIME_MINUTE)},
        {"FPS", std::to_string(GetFPS())},
    };

    impl::draw_label_pairs(info_box.with_padding_left(gfx::S).with_padding_top(
                               gfx::L + gfx::M * 2),
                           info, gfx::M, gfx::XS, 0, gfx::ORANGE_WARM,
                           gfx::YELLOW_WARM);

    // Legend panel
    gfx::Box legend_box = m_grid.span(0, 3, 6, 8)
                              .with_padding_left(gfx::S)
                              .with_padding_top(gfx::L)
                              .with_draw_background(BLACK);

    GuiGroupBox(legend_box.rectangle(), "Legend");

    gfx::Box legend_content =
        legend_box.with_padding_left(gfx::S).with_padding_top(gfx::L + gfx::M);

    DrawCircle(legend_content.x + 8, legend_content.y + 8, 6, gfx::YELLOW_WARM);
    DrawText("Kepler (Analytical)", legend_content.x + 25, legend_content.y,
             gfx::M, RAYWHITE);

    DrawCircle(legend_content.x + 8, legend_content.y + 30, 6, gfx::RED_WARM);
    DrawText("Euler", legend_content.x + 25, legend_content.y + 22, gfx::M,
             RAYWHITE);

    DrawCircle(legend_content.x + 8, legend_content.y + 52, 6, gfx::BLUE_WARM);
    DrawText("Verlet", legend_content.x + 25, legend_content.y + 44, gfx::M,
             RAYWHITE);

    // Controls panel
    gfx::Box controls_box = m_grid.span(0, 3, 9, 11)
                                .with_padding_left(gfx::S)
                                .with_padding_top(gfx::L)
                                .with_draw_background(BLACK);

    GuiGroupBox(controls_box.rectangle(), "Controls");

    std::vector<std::pair<std::string, std::string>> controls{
        {"[Space]",  "Pause/Resume"},
        {"[H]",      "Toggle UI"   },
        {"[WASD]",   "Move Camera" },
        {"[Scroll]", "Zoom"        },
        {"[= / -]",  "Time Factor" },
        {"[[ / ]]",  "Zoom In/Out" }
    };

    impl::draw_label_pairs(
        controls_box.with_padding_left(gfx::S).with_padding_top(gfx::L +
                                                                gfx::M * 2),
        controls, gfx::M, gfx::XS, 0, gfx::ORANGE_WARM, gfx::YELLOW_WARM);
}

}  // namespace nbody::scenario
