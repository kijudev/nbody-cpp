#include <cmath>
#include <iostream>

#include "body.hpp"
#include "raylib.h"
#include "simulation.hpp"

int main() {
    const int win_width = 800;
    const int win_height = 600;
    sim::Simulation simulation;

    simulation.bodies.push_back(sim::Body({100, 100}, {0, 0}, 1000.0f));
    simulation.bodies.push_back(sim::Body({400, 400}, {0, 0}, 2000.0f));

    InitWindow(win_width, win_height, "nbody-cpp");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        simulation.update();

        for (auto body : simulation.bodies) {
            body.update(GetFrameTime());
        }

        for (const auto body : simulation.bodies) {
            std::cout << "body: " << body.pos.x << " " << body.pos.y << "\n";
        }

        for (const auto body : simulation.bodies) {
            DrawCircle((int)body.pos.x, (int)body.pos.y, std::sqrt(body.mass),
                       WHITE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
