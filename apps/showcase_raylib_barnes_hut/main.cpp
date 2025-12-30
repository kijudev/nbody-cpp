#include <cmath>
#include <vector>
#include <algorithm>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "raylib.h"
#include "sim/body2.hpp"
#include "sim/integrator2.hpp"
#include "sim/sim2_linear_barnes_hut.hpp"


int main() {
    // Window
    const I32 screen_width  = 1024;
    const I32 screen_height = 768;

    InitWindow(screen_width, screen_height, "nbody - playground (sim2_linear_barnes_hut)");
    SetTargetFPS(60);

    // Type aliases
    using Float = float;
    using Vec2  = nbody::Vec2T<Float>;
    using Body  = nbody::Body2T<Float>;

    // Initial bodies (small configuration)
    std::vector<Body> bodies{
        Body::from_pos_mass(Vec2{1.0f, 1.0f}, 1.0f),
        Body::from_pos_mass(Vec2{-1.0f, -1.0f}, 1.0f),
        Body::from_pos_mass(Vec2{0.25f, 0.25f}, 1.0f),
        Body::from_pos_mass(Vec2{0.75f, 0.25f}, 1.0f),
    };

    nbody::Sim2LinearBarnesHut<Float> sim(bodies, &nbody::integrate2_euler<Float>);

    // Camera / view
    Vec2 cam_pos{0.0f, 0.0f};
    float zoom = 200.0f; // pixels per world unit (world ~1.0 becomes visible)
    const float zoom_step = 1.15f;

    bool paused = false;
    bool show_tree = true;
    float sim_speed = 1.0f;

    const Vector2 screen_center = {screen_width / 2.0f, screen_height / 2.0f};

    auto world_to_screen = [&](const Vec2& p) -> Vector2 {
        float sx = screen_center.x + (p.x - cam_pos.x) * zoom;
        float sy = screen_center.y + (p.y - cam_pos.y) * zoom;
        return Vector2{sx, sy};
    };

    // Build initial tree for visualization
    sim.tree_construct();

    while (!WindowShouldClose()) {
        // Input
        float wheel = GetMouseWheelMove();
        if (wheel > 0) zoom *= zoom_step;
        if (wheel < 0) zoom /= zoom_step;

        if (IsKeyPressed(KEY_UP)) zoom *= zoom_step;
        if (IsKeyPressed(KEY_DOWN)) zoom /= zoom_step;

        const float pan_speed = 20.0f / zoom;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) cam_pos.x -= pan_speed;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) cam_pos.x += pan_speed;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) cam_pos.y -= pan_speed;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) cam_pos.y += pan_speed;

        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyPressed(KEY_T)) show_tree = !show_tree;
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) sim_speed *= 2.0f;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) sim_speed *= 0.5f;
        if (IsKeyPressed(KEY_R)) {
            cam_pos = Vec2{0.0f, 0.0f};
            zoom = 200.0f;
        }

        float dt = GetFrameTime();
        if (!paused) {
            sim.step(static_cast<Float>(dt * sim_speed));
            // step currently rebuilds and frees the tree internally; rebuild for visualization
            sim.tree_construct();
        } else {
            // ensure tree exists while paused for interactive exploration
            sim.tree_construct();
        }

        // Draw
        BeginDrawing();
        ClearBackground(Color{0, 0, 0, 255});

        // faint axes
        DrawLineV({0.0f, screen_center.y}, {static_cast<float>(screen_width), screen_center.y},
                  Fade(Color{255, 255, 255, 255}, 0.06f));
        DrawLineV({screen_center.x, 0.0f}, {screen_center.x, static_cast<float>(screen_height)},
                  Fade(Color{255, 255, 255, 255}, 0.06f));

        // Draw Barnes-Hut quads if requested
        if (show_tree) {
            // Use the sim's draw_quads helper to iterate nodes
            sim.draw_quads([&](const Vec2& center, Float radius, bool is_leaf, Float mass, const Vec2& com) {
                (void)mass;
                // Convert center to screen
                Vector2 cs = world_to_screen(center);
                float size_pixels = static_cast<float>(2.0f * radius * zoom);
                float half = size_pixels * 0.5f;
                Rectangle rect = {cs.x - half, cs.y - half, size_pixels, size_pixels};

                // Color by whether leaf or internal
                Color col = is_leaf ? Fade(Color{0, 228, 48, 255}, 0.25f)
                                    : Fade(Color{0, 121, 241, 255}, 0.12f);
                DrawRectangleLinesEx(rect, 1, col);

                // Draw center-of-mass (small dot)
                Vector2 com_s = world_to_screen(com);
                DrawCircleV(com_s, std::max(1.0f, zoom * 0.01f), Color{230, 41, 55, 255});
            });
        }

        // Draw bodies
        const auto& sim_bodies = sim.bodies();
        for (size_t i = 0; i < sim_bodies.size(); ++i) {
            const Body& b = sim_bodies[i];
            Vector2 s = world_to_screen(b.pos);

            float radius = 4.0f;
            if (b.mass > 10.0f) radius = 8.0f + std::log2(b.mass) * 2.0f;
            radius = std::max(2.0f, radius * (zoom * 0.03f + 0.5f));

            DrawCircleV(s, radius, Color{255, 255, 255, 255});
        }

        // HUD
        DrawFPS(10, 10);
        DrawText(paused ? "PAUSED (SPACE to toggle)" : "RUNNING (SPACE to toggle)", 10, 30, 14, Color{255, 255, 255, 255});
        DrawText("Toggle tree: T | Zoom: mouse wheel or UP/DOWN | Pan: WASD / arrows", 10, 50, 14, Color{255, 255, 255, 255});
        DrawText("Sim speed: +/- (keyboard). Reset view: R", 10, 70, 14, Color{255, 255, 255, 255});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
