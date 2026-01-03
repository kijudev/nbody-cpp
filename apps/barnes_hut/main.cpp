#include <raylib.h>

#include <cmath>
#include <span>
#include <vector>

#include "base/type.hpp"
#include "nbody2/barnes_hut.hpp"
#include "nbody2/const.hpp"
#include "nbody2/euler.hpp"
#include "nbody2/generators.hpp"
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
        camera.target.y -= 10.0 / camera.zoom;
    }
    if (IsKeyDown(KEY_S)) {
        camera.target.y += 10.0 / camera.zoom;
    }
    if (IsKeyDown(KEY_A)) {
        camera.target.x -= 10.0 / camera.zoom;
    }
    if (IsKeyDown(KEY_D)) {
        camera.target.x += 10.0 / camera.zoom;
    }

    camera.zoom = expf(logf(camera.zoom) + (static_cast<F32>(GetMouseWheelMove() * 0.1)));
}

Color color_lerp(Color a, Color b, float value) {
    value = std::clamp<F32>(value, 0.0, 1.0);

    return Color{(U8)(a.r + value * (b.r - a.r)), (U8)(a.g + value * (b.g - a.g)),
                 (U8)(a.b + value * (b.b - a.b)), 255};
}

Color star_color(F32 mass) {
    F32 m_red    = 5'000.0;
    F32 m_yellow = 15'000.0;
    F32 m_white  = 25'000.0;
    F32 m_blue   = 50'000.0;

    Color c_red    = {255, 60, 60, 255};
    Color c_orange = {255, 180, 60, 255};
    Color c_yellow = {255, 255, 200, 255};
    Color c_blue   = {100, 200, 255, 255};
    Color c_violet = {180, 180, 255, 255};

    if (mass <= m_red) return c_red;

    if (mass < m_yellow) {
        F32 t = (mass - m_red) / (m_yellow - m_red);
        return color_lerp(c_red, c_orange, t);
    }

    if (mass < m_white) {
        F32 t = (mass - m_yellow) / (m_white - m_yellow);
        return color_lerp(c_yellow, c_blue, t);
    }

    if (mass < m_blue) {
        F32 t = (mass - m_white) / (m_blue - m_white);
        return color_lerp(c_blue, c_violet, t);
    }

    return c_violet;
}

void draw_bodies(std::span<const nbody2::BodyT<F32>, std::dynamic_extent> bodies) {
    for (const auto& body : bodies) {
        DrawCircleV(body.pm.pos.raylib_vector2(), std::sqrt(body.pm.mass),
                    star_color(body.pm.mass));
    }
}

void draw_nodes(const std::vector<const nbody2::SimBarnesHut<F32>::Node*>& nodes) {
    for (const auto& node : nodes) {
        F32 x = node->region_center.x - node->region_radius;
        F32 y = node->region_center.y - node->region_radius;

        DrawLineV({x, y}, {x + node->region_radius * 2, y}, {255, 255, 255, 20});
        DrawLineV({x, y}, {x, y + node->region_radius * 2}, {255, 255, 255, 20});
        DrawLineV({x + node->region_radius * 2, y},
                  {x + node->region_radius * 2, y + node->region_radius * 2}, {255, 255, 255, 20});
        DrawLineV({x, y + node->region_radius * 2},
                  {x + node->region_radius * 2, y + node->region_radius * 2}, {255, 255, 255, 20});
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
    nbody2::GenerateDistributionConfig<F32> generate_config{
        .n           = 3'000,
        .min_mass    = 1000.0,
        .max_mass    = 100'000.0,
        .radius      = 10'000.0,
        .position_fn = nbody2::generate_position_distribution_plummer_model<F32>,
        .mass_fn     = nbody2::generate_mass_distribution_salpeter_imf<F32>,
    };

    std::vector<nbody2::BodyT<F32>> bodies = nbody2::generate_distribution(generate_config);

    nbody2::SimBarnesHut<F32>::Config config{
        .bodies       = std::move(bodies),
        .integrate_fn = nbody2::euler_integrate_body<F32>,
        .g            = nbody2::G_TOY,
        .softening    = nbody2::SOFTENING_TOY,
    };

    nbody2::SimBarnesHut<F32> sim(config);

    I32 window_width  = 800;
    I32 window_height = 600;
    I32 target_fps    = 60;

    InitWindow(window_width, window_height, "nbody - direct");
    SetTargetFPS(target_fps);

    Camera2D camera = make_camera();

    // --- Main Loop ---
    while (!WindowShouldClose()) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        window_width  = GetScreenWidth();
        window_height = GetScreenHeight();

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
