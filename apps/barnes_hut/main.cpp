#include <raylib.h>

#include <cmath>
#include <span>
#include <vector>

#include "base/type.hpp"
#include "nbody2/barnes_hut.hpp"
#include "nbody2/const.hpp"
#include "nbody2/euler.hpp"
#include "nbody2/type.hpp"

Camera2D make_camera() {
    Camera2D camera = {
        .offset   = {0.0, 0.0},
        .target   = {0.0, 0.0},
        .rotation = 0.0,
        .zoom     = 1.0,
    };

    return camera;
}

void handle_camera_controls(Camera2D& camera) {
    if (IsKeyDown(KEY_W)) {
        camera.target.y -= 1.0;
    }
    if (IsKeyDown(KEY_S)) {
        camera.target.y += 1.0;
    }
    if (IsKeyDown(KEY_A)) {
        camera.target.x -= 1.0;
    }
    if (IsKeyDown(KEY_D)) {
        camera.target.x += 1.0;
    }

    camera.zoom = expf(logf(camera.zoom) + (static_cast<F32>(GetMouseWheelMove() * 0.1)));
}

void draw_bodies(std::span<const nbody2::BodyT<F32>, std::dynamic_extent> bodies) {
    for (const auto& body : bodies) {
        DrawCircleV(body.pm.pos.raylib_vector2(), sqrt(body.pm.mass), WHITE);
    }
}

void draw_nodes(const std::vector<const nbody2::SimBarnesHut<F32>::Node*>& nodes) {
    for (const auto& node : nodes) {
        F32 x = node->region_center.x - node->region_radius;
        F32 y = node->region_center.y - node->region_radius;
        DrawRectangleLines(x, y, node->region_radius * 2, node->region_radius * 2,
                           {255, 255, 255, 100});
    }
}

void draw_ui(I32 window_width, I32 window_height) {
    constexpr I32 unit = 8;

    std::string fps_text = "FPS: " + std::to_string(GetFPS());

    DrawRectangleLines(unit, unit, unit * 16, unit * 16, WHITE);
    DrawText(fps_text.c_str(), unit * 2, unit * 2, unit, WHITE);

    (void)window_width;
    (void)window_height;
}

int main() {
    std::vector<nbody2::BodyT<F32>> bodies;

    nbody2::BodyT<F32> a = {
        .pm  = {{10.0, 10.0}, 1.0},
        .vel = {0.0,          0.0},
        .acc = {0.0,          0.0},
    };

    nbody2::BodyT<F32> b = {
        .pm  = {{-10.0, -10.0}, 1.0},
        .vel = {0.0,            0.0},
        .acc = {0.0,            0.0},
    };

    nbody2::BodyT<F32> c = {
        .pm  = {{-3.0, 24.0}, 1.0},
        .vel = {0.0,          0.0},
        .acc = {0.0,          0.0},
    };

    nbody2::SimBarnesHut<F32>::Config config{
        .bodies       = {a, b, c},
        .integrate_fn = nbody2::euler_integrate_body<F32>,
        .g            = nbody2::G_TOY,
        .softening    = nbody2::SOFTENING_TOY,
    };

    nbody2::SimBarnesHut<F32> sim(config);

    I32 window_height = 800;
    I32 window_width  = 600;
    I32 target_fps    = 60;

    InitWindow(window_width, window_height, "nbody - direct");
    SetTargetFPS(target_fps);

    Camera2D camera = make_camera();

    // --- Main Loop ---
    while (!WindowShouldClose()) {
        F32 dt = GetFrameTime();
        sim.step(dt * 10);

        handle_camera_controls(camera);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        draw_nodes(sim.collect_nodes());
        draw_bodies(sim.bodies());
        EndMode2D();

        draw_ui(window_width, window_height);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
