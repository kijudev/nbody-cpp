#include <raylib.h>

#include "gfx/box.hpp"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "base/type.hpp"
#include "gfx/grid.hpp"
#include "gfx/window.hpp"
#include "scenario/bhm_40k.hpp"

using namespace nbody::base::type;
using namespace nbody;

enum class AppState {
    Menu,
    BHM40K,
};

int main() {
    constexpr I32 TARGET_FPS = 60;

    gfx::Window window{
        .width  = 800,
        .height = 600,
    };

    window.init();
    GuiLoadStyle("theme/raygui_style.rgs");
    SetTargetFPS(TARGET_FPS);

    AppState app_state = AppState::Menu;

    scenario::BHM40K bhm40k_scenario;

    std::vector<std::string> scenarios = {
        "Barnes-Hut algorithm with Morton ordering for 40K particles"};

    gfx::Grid menu_grid{
        .width  = window.width,
        .height = window.height,
        .cols   = 12,
        .rows   = 16,
    };

    gfx::Grid scenario_grid{
        .width  = window.width,
        .height = window.height,
        .cols   = 12,
        .rows   = 16,
    };

    while (!WindowShouldClose()) {
        window.handle_resize();
        menu_grid.width      = window.width;
        menu_grid.height     = window.height;
        scenario_grid.width  = window.width;
        scenario_grid.height = window.height;

        BeginDrawing();
        ClearBackground(BLACK);

        if (app_state == AppState::Menu) {
            DrawText("Select Scenario", 32, 32, 32, WHITE);

            for (USize i = 0; i < scenarios.size(); ++i) {
                gfx::Box box =
                    menu_grid.span(0, 4, i + 2, i + 2).with_padding(8).with_draw_border(WHITE);

                if (GuiButton(box.rectangle(), scenarios[i].c_str())) {
                    switch (i) {
                        case 0:
                            bhm40k_scenario.init(window);
                            app_state = AppState::BHM40K;
                            break;
                    }
                }
            }
        } else if (app_state == AppState::BHM40K) {
            bhm40k_scenario.step(window);

            gfx::Box back_box =
                scenario_grid.span(scenario_grid.cols - 1, scenario_grid.cols - 1, 0, 0)
                    .with_padding(12)
                    .with_draw_border(WHITE);

            if (GuiButton(back_box.rectangle(), "Back to Menu")) {
                app_state = AppState::Menu;
            }
        }

        EndDrawing();
    }

    return 0;
}
