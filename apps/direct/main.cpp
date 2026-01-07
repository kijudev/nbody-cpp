#include <raylib.h>

#include <cmath>
#include <cstdlib>
#include <span>
#include <string>
#include <vector>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "gfx/draw.hpp"
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
        .radius      = nbody::sim::scale_au::UNIT_AU * 20,
        .position_fn = nbody::sim::generate_position_distribution_plummer_model<Float>,
        .mass_fn     = nbody::sim::generate_mass_distribution_salpeter_imf<Float>,
    };

    std::vector<Body> bodies = nbody::sim::generate_distribution(generate_distribution_config);

    nbody::sim::Direct<Float>::Config sim_config{
        .bodies       = std::move(bodies),
        .integrate_fn = nbody::sim::integrate_body_verlet<Float>,
        .g            = nbody::sim::scale_au::CONST_G,
        .softening    = nbody::sim::scale_au::CONST_SOFTENING,
    };

    nbody::sim::Direct<Float> sim(sim_config);

    I32 screen_width  = 800;
    I32 screen_height = 600;
    I32 target_fps    = 60;

    // NOTE: This is a little bit artificial. The scale_factor scales only the bodies in the
    // visualisatoin.
    Float scale_factor = nbody::sim::scale_au::UNIT_AU * 100.0;

    // NOTE: How much time passes per second irl.
    Float time_factor = nbody::sim::scale_au::TIME_YEAR * 100.0;

    bool is_running = true;

    nbody::gfx::Camera<Float> camera{
        .screen_width   = screen_width,
        .screen_height  = screen_height,
        .zoom           = 1.0,
        .movement_speed = nbody::sim::scale_au::UNIT_AU / nbody::sim::scale_au::TIME_MINUTE,
        .scaling_speed  = 1.0,
    };

    InitWindow(screen_width, screen_height, "NBody - Direct");
    SetTargetFPS(target_fps);

    while (!WindowShouldClose()) {
        Float dt = static_cast<Float>(GetFrameTime());

        camera.handle_controls(dt);

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

        std::span<const Body, std::dynamic_extent> bodies = sim.bodies();

        for (USize i = 0; i < sim.bodies().size(); ++i) {
            const Body body   = bodies[i];
            const Vec2 center = camera.world_to_screen_vec(body.pm.pos);
            const Vec2 edge   = camera.world_to_screen_vec(
                Vec2{body.pm.pos.x + std::cbrt(body.pm.mass), body.pm.pos.y});

            DrawCircleV(center.as_raylib_vector(), center.distance(edge) * scale_factor, WHITE);
        }

        nbody::gfx::draw_ruler(camera, " AU");

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
