#include <cmath>

#include "body.hpp"
#include "raylib.h"
#include "simulation.hpp"

int main() {
    const int win_width = 800;
    const int win_height = 600;
    sim::Simulation simulation;

    simulation.bodies.push_back(sim::Body({0, 0}, {0, 0}, 10.0f));

    InitWindow(win_width, win_height, "nbody-cpp");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        for (const auto body : simulation.bodies) {
            DrawCircle((int)body.pos.x, (int)body.pos.y, body.mass, WHITE);
        }

        for (auto& body : simulation.bodies) {
            body.pos.x += 1.0f;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
