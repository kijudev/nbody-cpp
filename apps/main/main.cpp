#include <iostream>

#include "raylib.h"

int main() {
    const int win_width = 800;
    const int win_height = 600;

    InitWindow(win_width, win_height, "nbody-cpp");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawText("N Body Problem simulation", 100, 100, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
