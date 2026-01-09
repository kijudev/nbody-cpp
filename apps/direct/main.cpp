#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <span>
#include <string>
#include <vector>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/draw.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"
#include "math/vec.hpp"
#include "sim/const.hpp"
#include "sim/direct.hpp"
#include "sim/generator.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

using namespace nbody::base::type;
using Float = F64;
using Body  = nbody::sim::BodyT<Float>;
using Vec2  = nbody::math::Vec2T<Float>;

int main() {
    nbody::sim::GenerateDistributionConfig<Float> generate_distribution_config{
        .n           = 1000,
        .min_mass    = nbody::sim::scale_au::MASS_HYGIEA,
        .max_mass    = nbody::sim::scale_au::MASS_SOL * 10,
        .radius      = nbody::sim::scale_au::DISTANCE_AU * 20,
        .position_fn = nbody::sim::generate_position_distribution_plummer_model<Float>,
        .mass_fn     = nbody::sim::generate_mass_distribution_salpeter_imf<Float>,
    };

    std::vector<Body> bodies = nbody::sim::generate_distribution(generate_distribution_config);

    nbody::sim::Direct<Float>::Config sim_config{
        .bodies       = std::move(bodies),
        .integrate_fn = nbody::sim::integrate_body_verlet<Float>,
        .g            = nbody::sim::scale_au::G,
        .softening    = nbody::sim::scale_au::SOFTENING,
    };

    nbody::sim::Direct<Float> sim(sim_config);

    I32 target_fps = 60;

    // NOTE: This is a little bit artificial. The scale_factor scales only the bodies in the
    // visualisatoin.
    Float scale_factor = nbody::sim::scale_au::DISTANCE_AU * 100.0;

    // NOTE: How much time passes per second irl.
    Float time_factor = nbody::sim::scale_au::TIME_YEAR * 100.0;

    bool is_running = true;

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

        I32 font_small   = std::clamp(8, std::min(window.width, window.height) / 96, 12);
        I32 font_regular = std::clamp(12, std::min(window.width, window.height) / 48, 16);
        I32 font_big     = std::clamp(16, std::min(window.width, window.height) / 32, 32);

        (void)font_small;

        if (is_running) {
            sim.step(dt * time_factor);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // --- NBody ---
        std::span<const Body, std::dynamic_extent> bodies = sim.bodies();
        for (USize i = 0; i < sim.bodies().size(); ++i) {
            const Body body   = bodies[i];
            const Vec2 center = camera.world_to_screen_vec(body.pm.pos);
            const Vec2 edge   = camera.world_to_screen_vec(
                Vec2{body.pm.pos.x + std::cbrt(body.pm.mass), body.pm.pos.y});
            Float r = center.distance(edge) * scale_factor;

            if (r > 1.0) {
                DrawCircleV(center.as_raylib_vector(), r, WHITE);
            }
        }

        // --- UI ---
        nbody::gfx::ui_draw_text(grid.row(0, 3)
                                     .with_padding(16)
                                     .with_border(WHITE, 0)
                                     .with_background(BLACK, 0)
                                     .with_padding_left(8)
                                     .with_padding_y(8),
                                 nbody::gfx::Layout::CenterLeft, font_big, "NBody");

        nbody::gfx::ui_draw_text(grid.span(0, 1, 2, 2)
                                     .with_padding(16)
                                     .with_border(WHITE, 0)
                                     .with_background(BLACK, 0)
                                     .with_padding_left(8)
                                     .with_padding_y(8),
                                 nbody::gfx::Layout::TopLeft, font_regular,
                                 "FPS: " + std::to_string(GetFPS()) + "\n" +
                                     "Bodies: " + std::to_string(sim.bodies().size()));

        nbody::gfx::ui_draw_ruler(camera, " AU");

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
