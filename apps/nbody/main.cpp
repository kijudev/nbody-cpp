#include <raylib.h>
#include "base/type.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"

using namespace nbody::base::type;
using namespace nbody;

int main() {
    constexpr I32 TARGET_FPS = 60;

    gfx::Window window{
        .width  = 800,
        .height = 600,
        .title  = "NBody Simulation",
    };

    gfx::Grid ui_menu_grid{
        .width  = window.width,
        .height = window.height,
        .cols   = 12,
        .rows   = 16,
    };

    window.init();
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose()) {
        window.handle_resize();
    }

    return 0;
}
