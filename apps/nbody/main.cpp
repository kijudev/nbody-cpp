#include <raylib.h>

#include "gfx/box.hpp"
#include "scenario/barnes_hut_morton_plummer.hpp"
#include "scenario/barnes_hut_plummer.hpp"
#include "scenario/kepler_euler_verlet.hpp"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "base/type.hpp"
#include "gfx/const.hpp"
#include "gfx/draw.hpp"
#include "gfx/grid.hpp"
#include "gfx/layout.hpp"
#include "gfx/window.hpp"

using namespace nbody::base::type;
using namespace nbody;

enum class AppState {
    Menu,
    BarnesHutPlummer,
    BarnesHutMortonPlummer,
    KeplerEulerVerlet,
};

struct ScenarioInfo {
    std::string name;
    std::string description;
    AppState    state;
};

int main() {
    constexpr I32 TARGET_FPS = 60;

    gfx::Window window{
        .width  = 1920,
        .height = 1080,
        .title  = "N-Body Simulation",
    };

    window.init();
    GuiLoadStyle("theme/raygui_style.rgs");
    SetTargetFPS(TARGET_FPS);

    AppState app_state = AppState::Menu;

    scenario::BarnesHutPlummer       barnes_hut_plummer_scenario;
    scenario::BarnesHutMortonPlummer barnes_hut_morton_plummer_scenario;
    scenario::KeplerEulerVerlet      kepler_euler_verlet_scenario;

    std::vector<ScenarioInfo> scenarios = {
        {
         .name        = "Barnes-Hut Plummer",
         .description = "Barnes-Hut with the plummer model",
         .state       = AppState::BarnesHutPlummer,
         },
        {
         .name = "Barnes-Hut Morton",
         .description =
                "Barnes-Hut with the plummer model and Morton ordering",                                                                                                                    .state = AppState::BarnesHutMortonPlummer,
         },
        {
         .name        = "Kepler vs Euler vs Verlet",
         .description = "Compare analytical (Kepler), Euler, and Verlet "
                           "integrators for a two-body orbit",.state       = AppState::KeplerEulerVerlet,
         },
    };

    gfx::Grid main_grid{
        .width  = window.width,
        .height = window.height,
        .cols   = 24,
        .rows   = 24,
    };

    while (!WindowShouldClose()) {
        window.handle_resize();
        main_grid.width  = window.width;
        main_grid.height = window.height;

        BeginDrawing();
        ClearBackground(BLACK);

        if (app_state == AppState::Menu) {
            gfx::Box title_box = main_grid.span(0, main_grid.cols - 1, 2, 3)
                                     .with_padding(gfx::M);
            gfx::draw_text(title_box, gfx::Layout::Center, gfx::XXL,
                           "N-Body Simulation", WHITE);

            gfx::Box subtitle_box = main_grid.span(0, main_grid.cols - 1, 4, 4)
                                        .with_padding(gfx::S);
            gfx::draw_text(subtitle_box, gfx::Layout::Center, gfx::L,
                           "Select a scenario to begin", WHITE);

            constexpr I32 card_start_row = 6;
            constexpr I32 card_height    = 3;
            constexpr I32 card_spacing   = 1;
            constexpr I32 card_col_start = 6;
            constexpr I32 card_col_end   = 18;

            for (USize i = 0; i < scenarios.size(); ++i) {
                const I32 row_start =
                    card_start_row + i * (card_height + card_spacing);
                const I32 row_end = row_start + card_height - 1;

                gfx::Box card_box = main_grid.span(card_col_start, card_col_end,
                                                   row_start, row_end);
                DrawRectangleLinesEx(card_box.rectangle(), 2, WHITE);

                gfx::Box content_box = card_box.with_padding(gfx::M);
                gfx::draw_text(content_box, gfx::Layout::CenterLeft, gfx::L,
                               scenarios[i].name, WHITE);

                gfx::Box button_box = main_grid
                                          .span(card_col_end - 3, card_col_end,
                                                row_start, row_end)
                                          .with_padding(gfx::S);

                if (GuiButton(button_box.rectangle(), "Start")) {
                    switch (scenarios[i].state) {
                        case AppState::BarnesHutPlummer:
                            barnes_hut_plummer_scenario.init(window);
                            app_state = AppState::BarnesHutPlummer;
                            break;
                        case AppState::BarnesHutMortonPlummer:
                            barnes_hut_morton_plummer_scenario.init(window);
                            app_state = AppState::BarnesHutMortonPlummer;
                            break;
                        case AppState::KeplerEulerVerlet:
                            kepler_euler_verlet_scenario.init(window);
                            app_state = AppState::KeplerEulerVerlet;
                            break;
                        default:
                            break;
                    }
                }
            }

            gfx::Box footer_box =
                main_grid
                    .span(0, main_grid.cols - 1, main_grid.rows - 1,
                          main_grid.rows - 1)
                    .with_padding(gfx::S);
            gfx::draw_text(footer_box, gfx::Layout::BottomCenter, gfx::S,
                           "Press ESC to exit", GRAY);

        } else if (app_state == AppState::BarnesHutPlummer) {
            barnes_hut_plummer_scenario.step(window);
        } else if (app_state == AppState::BarnesHutMortonPlummer) {
            barnes_hut_morton_plummer_scenario.step(window);
        } else if (app_state == AppState::KeplerEulerVerlet) {
            kepler_euler_verlet_scenario.step(window);
        }

        if (app_state != AppState::Menu) {
            gfx::Box back_button_box =
                main_grid.span(main_grid.cols - 3, main_grid.cols - 1, 0, 1)
                    .with_padding(gfx::S);

            if (GuiButton(back_button_box.rectangle(), "Menu")) {
                app_state = AppState::Menu;
            }
        }

        EndDrawing();
    }

    return 0;
}
