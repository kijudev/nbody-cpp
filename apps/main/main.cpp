#include <cmath>
#include <cstddef>

#include "body.hpp"
#include "raylib.h"
#include "simulation.hpp"

int main() {
    const int win_width = 1200;
    const int win_height = 800;
    sim::Simulation simulation;

    for (size_t i = 0; i < 100; ++i) {
        simulation.place_random_body();
    }

    simulation.place_body(
        sim::Body({win_width / 2.0, win_height / 2.0}, {0.0, 0.0}, 1000'000.0));

    simulation.place_body(sim::Body(
        {win_width / 2.0 + 200.0, win_height / 2.0 + 0.0}, {0.0, 50.0}, 10000));

    simulation.place_body(sim::Body(
        {win_width / 2.0 + 400.0, win_height / 2.0 + 0.0}, {0.0, 35.0}, 10000));

    InitWindow(win_width, win_height, "nbody-cpp");
    SetTargetFPS(60);

    Camera2D camera;
    camera.target = (Vector2){0.0, 0.0};
    camera.offset = (Vector2){0.0, 0.0};
    camera.rotation = 0.0;
    camera.zoom = 1.0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        camera.offset =
            (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};

        camera.zoom =
            expf(logf(camera.zoom) + ((float)GetMouseWheelMove() * 0.1f));

        if (IsKeyDown(KEY_W)) {
            camera.target.y += 150.0f * dt;
        }
        if (IsKeyDown(KEY_S)) {
            camera.target.y -= 150.0f * dt;
        }
        if (IsKeyDown(KEY_D)) {
            camera.target.x += 150.0f * dt;
        }
        if (IsKeyDown(KEY_A)) {
            camera.target.x -= 150.0f * dt;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
        simulation.update();

        for (auto& body : simulation.bodies) {
            body.update(dt);
        }

        for (const auto body : simulation.bodies) {
            DrawCircle((int)body.pos.x, (int)body.pos.y,
                       std::max(std::sqrt(body.mass / 1000.0), 1.0), WHITE);
        }

        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
