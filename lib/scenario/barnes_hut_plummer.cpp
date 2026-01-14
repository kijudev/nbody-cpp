#include "scenario/barnes_hut_plummer.hpp"

#include <raygui.h>
#include <raylib.h>

#include <cstring>
#include <format>
#include <vector>

#include "gfx/box.hpp"
#include "gfx/const.hpp"
#include "gfx/draw.hpp"
#include "gfx/window.hpp"
#include "scenario/draw.hpp"
#include "scenario/impl.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/const.hpp"
#include "sim/generator.hpp"

namespace nbody::scenario {
using namespace nbody::base::type;

void BarnesHutPlummer::init(const gfx::Window& window) {
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

    sim::BarnesHut<Float>::Config sim_config{
        .bodies       = std::move(bodies),
        .g            = sim::scale_au::G,
        .softening    = sim::scale_au::SOFTENING,
        .parallel     = true,
        .integrate_fn = sim::integrate_body_verlet<Float>,
    };

    m_sim = sim::BarnesHut<Float>(sim_config);

    m_camera = {
        .screen_width   = window.width,
        .screen_height  = window.height,
        .zoom           = 0.1,
        .movement_speed = sim::scale_au::DISTANCE_AU * 200.0,
        .scaling_speed  = 10.0,
    };

    m_grid = {
        .width  = window.width,
        .height = window.height,
        .cols   = 16,
        .rows   = 12,
    };
}

void BarnesHutPlummer::step(const gfx::Window& window) {
    Float dt = static_cast<Float>(GetFrameTime());

    handle_input();
    handle_slingshot_input(dt);
    update_sim(dt);
    update_camera(dt, window);
    draw_sim();
    draw_slingshot();
    draw_ui(window);
}

void BarnesHutPlummer::handle_input() {
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

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsKeyDown(KEY_LEFT_SHIFT)) {
        auto maybe_idx =
            impl::get_body_at_mouse_position_au(m_camera, m_sim.bodies());
        if (maybe_idx.has_value()) {
            m_tracked_body_index = maybe_idx.value();
            m_is_tracking_body   = true;
        }
    }
}

void BarnesHutPlummer::handle_slingshot_input(Float dt) {
    (void)dt;

    bool was_active = m_slingshot_state.is_active;

    impl::handle_slingshot_input(m_slingshot_state, m_camera);

    if (was_active && !m_slingshot_state.is_active) {
        Float drag_distance =
            m_slingshot_state.current_pos.distance(m_slingshot_state.start_pos);

        if (drag_distance > 0.01) {
            launch_slingshot_body();
        }
    }
}

void BarnesHutPlummer::update_sim(Float dt) {
    if (!m_is_sim_running) {
        return;
    }

    Float sim_dt = dt * m_time_factor;
    m_sim.step(sim_dt);
    m_simulation_time += sim_dt;
}

void BarnesHutPlummer::update_camera(Float dt, const gfx::Window& window) {
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

void BarnesHutPlummer::draw_sim() {
    if (!m_is_sim_visible) {
        return;
    }

    impl::draw_bodies_monocolor(m_camera, m_scale_factor, m_sim.bodies(),
                                WHITE);
}

void BarnesHutPlummer::draw_slingshot() {
    impl::draw_slingshot(m_slingshot_state, m_camera, m_scale_factor);
}

void BarnesHutPlummer::launch_slingshot_body() {
    Body new_body = impl::create_slingshot_body(m_slingshot_state, m_camera);
    m_sim.insert_body(std::move(new_body));
}

void BarnesHutPlummer::draw_ui(const gfx::Window& window) {
    if (!m_is_ui_visible) {
        return;
    }

    m_grid.width  = window.width;
    m_grid.height = window.height;

    impl::draw_cross_center(m_camera, gfx::L, 2, gfx::YELLOW_WARM);
    impl::draw_ruler_au(m_camera, gfx::XL, gfx::XL, 0.5, gfx::YELLOW_WARM);

    gfx::Box general_info_box = m_grid.span(0, 2, 0, 2)
                                    .with_padding_left(gfx::S)
                                    .with_padding_top(gfx::L)
                                    .with_draw_background(BLACK);

    gfx::Box simulation_info_box = m_grid.span(0, 2, 3, 5)
                                       .with_padding_left(gfx::S)
                                       .with_padding_top(gfx::L)
                                       .with_draw_background(BLACK);

    gfx::Box body_info_box = m_grid.span(0, m_is_tracking_body ? 3 : 2, 6, 7)
                                 .with_padding_left(gfx::S)
                                 .with_padding_top(gfx::L)
                                 .with_draw_background(BLACK);

    gfx::Box control_info_box = m_grid.span(0, 2, 8, 11)
                                    .with_padding_left(gfx::S)
                                    .with_padding_top(gfx::L)
                                    .with_padding_bottom(gfx::L)
                                    .with_draw_background(BLACK);

    GuiGroupBox(general_info_box.rectangle(), "General Info");
    GuiGroupBox(simulation_info_box.rectangle(), "Simulation Info");
    GuiGroupBox(body_info_box.rectangle(), "Body Info");
    GuiGroupBox(control_info_box.rectangle(), "Control Info");

    gfx::draw_text(
        general_info_box.with_padding_left(gfx::S).with_padding_top(gfx::M),
        gfx::Layout::TopLeft, gfx::L, "Barnes-Hut Plummer", gfx::YELLOW_WARM);

    std::vector<std::pair<std::string, std::string>> general_info{
        {"Particles", std::to_string(m_sim.bodies().size())},
        {"Status", m_is_sim_running ? "Running" : "Paused"},
        {"Sim Time",
         std::format("{:.2f} years",
         m_simulation_time / sim::scale_au::TIME_YEAR)}
    };
    impl::draw_label_pairs(
        general_info_box.with_padding_left(gfx::S).with_padding_top(gfx::L +
                                                                    gfx::M * 2),
        general_info, gfx::M, gfx::XS, 0, gfx::YELLOW_PALE, gfx::YELLOW_WARM);

    std::vector<std::pair<std::string, std::string>> simulation_info{
        {"Time Factor",
         impl::format_time(m_time_factor, sim::scale_au::TIME_YEAR,
         sim::scale_au::TIME_DAY, sim::scale_au::TIME_HOUR,
         sim::scale_au::TIME_MINUTE)},
        {"Scale Factor", std::format("{:.2f}", m_scale_factor)},
        {"FPS", std::to_string(GetFPS())},
        {"Integrator", "Verlet"},
    };
    impl::draw_label_pairs(
        simulation_info_box.with_padding_left(gfx::S).with_padding_top(gfx::M),
        simulation_info, gfx::M, gfx::XS, 0, gfx::YELLOW_PALE,
        gfx::YELLOW_WARM);

    // --- Body Info ---
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
        impl::draw_label_pairs(
            body_info_box.with_padding_left(gfx::S).with_padding_top(gfx::M),
            body_info, gfx::M, gfx::XS, 0, gfx::YELLOW_PALE, gfx::YELLOW_WARM);
    } else {
        std::vector<std::pair<std::string, std::string>> body_info{
            {"Not tracking any body.", ""},
            {"Click a body to track.", ""}
        };
        impl::draw_label_pairs(
            body_info_box.with_padding_left(gfx::S).with_padding_top(gfx::M),
            body_info, gfx::M, gfx::XS, 0, gfx::YELLOW_PALE, gfx::YELLOW_WARM);
    }

    // --- Control Info ---
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

    impl::draw_label_pairs(
        control_info_box.with_padding_left(gfx::S).with_padding_top(gfx::M),
        control_info, gfx::M, gfx::XXS, -gfx::M, gfx::YELLOW_PALE,
        gfx::YELLOW_WARM);
}
}  // namespace nbody::scenario
