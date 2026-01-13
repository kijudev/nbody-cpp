#include "scenario/bhm_40k.hpp"

#include <raygui.h>
#include <raylib.h>

#include <cstring>
#include <format>
#include <vector>

#include "base/type.hpp"
#include "gfx/box.hpp"
#include "gfx/draw.hpp"
#include "gfx/window.hpp"
#include "scenario/draw.hpp"
#include "scenario/impl.hpp"
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

    Color property_color = Color{255, 180, 120, 255};  // NOTE: Warm orange.
    Color value_color    = Color{255, 230, 180, 255};  // NOTE: Warm yellow.
    Color header_color   = WHITE;

    gfx::Box general_info_box = m_grid.span(0, 2, 0, 2)
                                    .with_padding_left(12)
                                    .with_padding_top(24)
                                    .with_draw_background(BLACK);

    gfx::Box simulation_info_box = m_grid.span(0, 2, 3, 5)
                                       .with_padding_left(12)
                                       .with_padding_top(24)
                                       .with_draw_background(BLACK);

    gfx::Box body_info_box = m_grid.span(0, 2, 6, 7)
                                 .with_padding_left(12)
                                 .with_padding_top(24)
                                 .with_draw_background(BLACK);

    gfx::Box control_info_box = m_grid.span(0, 2, 8, 11)
                                    .with_padding_left(12)
                                    .with_padding_top(24)
                                    .with_padding_bottom(24)
                                    .with_draw_background(BLACK);

    GuiGroupBox(general_info_box.rectangle(), "General Info");
    GuiGroupBox(simulation_info_box.rectangle(), "Simulation Info");
    GuiGroupBox(body_info_box.rectangle(), "Body Info");
    GuiGroupBox(control_info_box.rectangle(), "Control Info");

    I32 y_offset = general_info_box.y + 32;
    I32 line_gap = 8;
    nbody::gfx::draw_text(
        general_info_box.with_padding_top(16).with_padding_left(12),
        nbody::gfx::Layout::TopLeft, 28, "Barnes-Hut Morton 40k", header_color);
    y_offset += 36;

    std::vector<std::pair<std::string, std::string>> general_info{
        {"Particles", std::to_string(m_sim.bodies().size())},
        {"Status", m_is_sim_running ? "Running" : "Paused"},
        {"Sim Time",
         std::format("{:.2f} years",
         m_simulation_time / sim::scale_au::TIME_YEAR)}
    };
    for (const auto& [prop, val] : general_info) {
        nbody::gfx::draw_text(gfx::Box{general_info_box.x + 16, y_offset,
                                       general_info_box.width - 20, 24},
                              nbody::gfx::Layout::TopLeft, 20, prop + ":",
                              property_color);
        nbody::gfx::draw_text(gfx::Box{general_info_box.x + 172, y_offset,
                                       general_info_box.width - 176, 24},
                              nbody::gfx::Layout::TopLeft, 20, val,
                              value_color);
        y_offset += 24 + line_gap;
    }

    // --- Simulation Info ---
    y_offset = simulation_info_box.y + 32;
    std::vector<std::pair<std::string, std::string>> simulation_info{
        {"Time Factor",
         nbody::scenario::impl::format_time(
             m_time_factor, sim::scale_au::TIME_YEAR, sim::scale_au::TIME_DAY,
         sim::scale_au::TIME_HOUR, sim::scale_au::TIME_MINUTE)},
        {"Scale Factor", std::format("{:.2f}", m_scale_factor)},
        {"FPS", std::to_string(GetFPS())},
        {"Integrator", "Verlet"},
    };
    for (const auto& [prop, val] : simulation_info) {
        nbody::gfx::draw_text(gfx::Box{simulation_info_box.x + 16, y_offset,
                                       simulation_info_box.width - 20, 24},
                              nbody::gfx::Layout::TopLeft, 18, prop + ":",
                              property_color);
        nbody::gfx::draw_text(gfx::Box{simulation_info_box.x + 152, y_offset,
                                       simulation_info_box.width - 156, 24},
                              nbody::gfx::Layout::TopLeft, 18, val,
                              value_color);
        y_offset += 22 + line_gap;
    }

    // --- Body Info ---
    y_offset = body_info_box.y + 32;
    if (m_is_tracking_body && m_tracked_body_index < m_sim.bodies().size()) {
        const Body& body = m_sim.bodies()[m_tracked_body_index];
        std::vector<std::pair<std::string, std::string>> body_info{
            {"Tracking Body", std::to_string(m_tracked_body_index)},
            {"Mass", std::format("{:.3e} kg", body.mass)},
            {"Position",
             std::format("({:.3e}, {:.3e}) AU", body.pos.x, body.pos.y)},
            {"Velocity",
             std::format("({:.3e}, {:.3e}) AU/yr", body.vel.x, body.vel.y)}
        };
        for (const auto& [prop, val] : body_info) {
            nbody::gfx::draw_text(gfx::Box{body_info_box.x + 16, y_offset,
                                           body_info_box.width - 20, 24},
                                  nbody::gfx::Layout::TopLeft, 18, prop + ":",
                                  property_color);
            nbody::gfx::draw_text(gfx::Box{body_info_box.x + 152, y_offset,
                                           body_info_box.width - 156, 24},
                                  nbody::gfx::Layout::TopLeft, 18, val,
                                  value_color);
            y_offset += 22 + line_gap;
        }
    } else {
        nbody::gfx::draw_text(gfx::Box{body_info_box.x + 16, y_offset,
                                       body_info_box.width - 20, 24},
                              nbody::gfx::Layout::TopLeft, 18,
                              "Not tracking any body.", property_color);
        y_offset += 22 + line_gap;
        nbody::gfx::draw_text(gfx::Box{body_info_box.x + 16, y_offset,
                                       body_info_box.width - 20, 24},
                              nbody::gfx::Layout::TopLeft, 18,
                              "Click a body to track.", value_color);
    }

    // --- Control Info ---
    y_offset = control_info_box.y + 32;
    std::vector<std::pair<std::string, std::string>> control_info{
        {"[Space]",       "Pause/Resume"        },
        {"[H]",           "Toggle UI"           },
        {"[Arrows/WASD]", "Move Camera"         },
        {"[Mouse Wheel]", "Zoom"                },
        {"[= / +]",       "Increase Time Factor"},
        {"[-]",           "Decrease Time Factor"},
        {"[[ / ]]",       "Scale Factor"        },
        {"[T]",           "Stop Tracking Body"  },
        {"[ESC]",         "Stop Tracking Body"  },
        {"[Click]",       "Track Body"          }
    };

    for (const auto& [key, desc] : control_info) {
        nbody::gfx::draw_text(
            gfx::Box{control_info_box.x + 16, y_offset, 120, 22},
            nbody::gfx::Layout::TopLeft, 16, key, property_color);
        nbody::gfx::draw_text(gfx::Box{control_info_box.x + 142, y_offset,
                                       control_info_box.width - 146, 22},
                              nbody::gfx::Layout::TopLeft, 16, desc,
                              value_color);
        y_offset += 20 + 2;
    }
}
}  // namespace nbody::scenario
