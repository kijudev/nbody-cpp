#include <cmath>
#include <cstddef>
#include <iostream>

#include "body.hpp"
#include "raylib.h"
#include "simulation.hpp"

int main() {
    const int win_width = 800;
    const int win_height = 600;
    sim::Simulation simulation;

    for (size_t i = 0; i < 200; ++i) {
        simulation.place_random_body();
    }

    simulation.place_body(sim::Body({400.0, 300.0}, {0.0, 0.0}, 1000'000.0));

    InitWindow(win_width, win_height, "nbody-cpp");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        BeginDrawing();

        ClearBackground(BLACK);

        simulation.update();

        for (auto& body : simulation.bodies) {
            body.update(dt);
        }

        for (const auto body : simulation.bodies) {
            DrawCircle((int)body.pos.x, (int)body.pos.y,
                       std::max(std::sqrt(body.mass / 1000.0), 1.0), WHITE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
