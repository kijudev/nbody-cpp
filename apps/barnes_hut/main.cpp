#include <raylib.h>

#include "base/type.hpp"
#include "gfx/camera.hpp"

using namespace nbody::base::type;
using Float = F64;

int main() {
    I32 screen_width  = 600;
    I32 screen_height = 800;
    I32 target_fps    = 60;

    nbody::gfx::Camera<Float> camera{
        .screen_width  = screen_width,
        .screen_height = screen_height,
    };

    SetTargetFPS(target_fps);

    while (!WindowShouldClose()) {
        Float dt = static_cast<Float>(GetFrameTime());

        BeginDrawing();

        ClearBackground(RAYWHITE);

        camera.handle_controls(dt);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
