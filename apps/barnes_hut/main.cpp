#include <raylib.h>

#include <cmath>
#include <cstdlib>
#include <span>
#include <string>
#include <vector>

#include "base/type.hpp"
#include "base/log.hpp"
#include "gfx/camera.hpp"
#include "gfx/draw.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"
#include "math/vec.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/const.hpp"
#include "sim/generator.hpp"
#include "sim/type.hpp"

using namespace nbody::base::type;
using Float = F32;
using Body  = nbody::sim::BodyT<Float>;
using Vec2  = nbody::math::Vec2T<Float>;

int main() {
    nbody::sim::GenerateDistributionConfig<Float> generate_distribution_config{
        .n           = 5000,
        .min_mass    = nbody::sim::scale_au::MASS_HYGIEA,
        .max_mass    = nbody::sim::scale_au::MASS_SOL * 10,
        .radius      = nbody::sim::scale_au::DISTANCE_AU * 10,
        .position_fn = nbody::sim::generate_position_distribution_plummer_model<Float>,
        .mass_fn     = nbody::sim::generate_mass_distribution_salpeter_imf<Float>,
    };

    std::vector<Body> bodies = nbody::sim::generate_distribution(generate_distribution_config);

    nbody::sim::BarnesHut<Float>::Config sim_config{
        .bodies       = std::move(bodies),
        .g            = nbody::sim::scale_au::G,
        .softening    = nbody::sim::scale_au::SOFTENING,
        .integrate_fn = nbody::sim::integrate_body_verlet<Float>,
    };

    nbody::sim::BarnesHut<Float> sim(sim_config);

    I32   target_fps   = 60;
    Float scale_factor = nbody::sim::scale_au::DISTANCE_AU * 100.0;
    Float time_factor  = nbody::sim::scale_au::TIME_YEAR * 100.0;
    bool  is_running   = true;

    nbody::gfx::Window window{
        .width  = 800,
        .height = 600,
    };

    nbody::gfx::Camera<Float> camera{
        .screen_width   = window.width,
        .screen_height  = window.height,
        .zoom           = 1.0,
        .movement_speed = nbody::sim::scale_au::DISTANCE_AU / nbody::sim::scale_au::TIME_MINUTE,
        .scaling_speed  = 1.0,
    };

    nbody::gfx::Grid grid{
        .width  = window.width,
        .height = window.height,
        .cols   = 16,
        .rows   = 12,
    };

    window.init();
    SetTargetFPS(target_fps);

    while (!WindowShouldClose()) {
        LOG_APP_DEBUG("GAME LOOP");
        Float dt = static_cast<Float>(GetFrameTime());

        window.handle_resize();
        camera.screen_width  = window.width;
        camera.screen_height = window.height;
        camera.handle_controls(dt);
        grid.width  = window.width;
        grid.height = window.height;

        if (window.width > 1000) {
            grid.cols = 24;
        } else {
            grid.cols = 16;
        }

        if (IsKeyDown(KEY_SPACE)) {
            is_running = false;
        } else {
            is_running = true;
        }

        if (IsKeyDown(KEY_F)) {
            time_factor = nbody::sim::scale_au::TIME_YEAR * 1000.0;
        } else {
            time_factor = nbody::sim::scale_au::TIME_YEAR * 100.0;
        }

        if (is_running) {
            sim.step(dt * time_factor);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // --- NBody ---
        nbody::gfx::draw_sim_bodies(camera, scale_factor, sim.bodies());

        // --- UI ---
        nbody::gfx::draw_ui_text(grid.row(0, 3)
                                     .with_padding(16)
                                     .with_draw_border(WHITE)
                                     .with_draw_background(BLACK)
                                     .with_padding_left(8)
                                     .with_padding_y(8),
                                 nbody::gfx::Layout::CenterLeft, 24, "NBody");

        nbody::gfx::draw_ui_text(grid.span(0, 1, 2, 2)
                                     .with_padding(16)
                                     .with_draw_border(WHITE)
                                     .with_draw_background(BLACK)
                                     .with_padding_left(8)
                                     .with_padding_y(8),
                                 nbody::gfx::Layout::TopLeft, 16,
                                 "FPS: " + std::to_string(GetFPS()) + "\n" +
                                     "Bodies: " + std::to_string(sim.bodies().size()));

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
