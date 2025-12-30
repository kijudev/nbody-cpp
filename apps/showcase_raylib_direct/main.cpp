#include <cmath>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "raylib.h"
#include "sim/const.hpp"
#include "sim/direct.hpp"
#include "sim/euler.hpp"
#include "sim/type.hpp"

int main() {
    const I32 screen_width  = 1024;
    const I32 screen_height = 768;

    InitWindow(screen_width, screen_height, "nbody - showcase (sim2_linear_direct)");
    SetTargetFPS(60);

    using Float = F32;
    using Vec2  = nbody::Vec2T<Float>;
    using Body  = nbody::Body2T<Float>;

    std::vector<Body> bodies;

    // NOTE: Initialize central body.
    Body central{};
    central.pos  = Vec2{0.0f, 0.0f};
    central.vel  = Vec2{0.0f, 0.0f};
    central.acc  = Vec2::zero();
    central.mass = static_cast<Float>(1000.0);

    // NOTE: Initialize orbiting body.
    const Float r = 200.0f;
    Body        orb{};
    orb.pos  = Vec2{r, 0.0f};
    orb.acc  = Vec2::zero();
    orb.mass = static_cast<Float>(1.0);

    // NOTE: Initialize orbiting body's velocity.
    const Float v = std::sqrt(static_cast<Float>(1.0) * central.mass / r);
    orb.vel       = Vec2{0.0f, v};

    bodies.push_back(central);
    bodies.push_back(orb);

    nbody::Sim2Direct<Float> sim(std::move(bodies), nbody::euler2<Float>, nbody::G_TOY,
                                 nbody::SOFTENING_TOY);

    // NOTE: Initialize camera/viewport controls.
    Vec2        cam_pos{0.0f, 0.0f};
    Float       zoom      = 1.0f;
    const Float zoom_step = 1.15f;
    bool        paused    = false;
    Float       sim_speed = 1.0f;

    const Vector2 screen_center = {screen_width / 2.0f, screen_height / 2.0f};

    while (!WindowShouldClose()) {
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
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) sim_speed *= 2.0f;
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) sim_speed *= 0.5f;

        float dt = GetFrameTime();
        if (!paused) {
            sim.step(static_cast<Float>(dt * sim_speed));
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawLineV({0.0f, screen_center.y}, {static_cast<float>(screen_width), screen_center.y},
                  Fade(WHITE, 0.06f));
        DrawLineV({screen_center.x, 0.0f}, {screen_center.x, static_cast<float>(screen_height)},
                  Fade(WHITE, 0.06f));

        auto world_to_screen = [&](const Vec2& p) -> Vector2 {
            float sx = screen_center.x + (p.x - cam_pos.x) * zoom;
            float sy = screen_center.y + (p.y - cam_pos.y) * zoom;
            return Vector2{sx, sy};
        };

        const auto& sim_bodies = sim.bodies();
        for (size_t i = 0; i < sim_bodies.size(); ++i) {
            const Body& b = sim_bodies[i];
            Vector2     s = world_to_screen(b.pos);

            float radius = 4.0f;
            if (b.mass > 10.0f) radius = 8.0f + std::log2(b.mass) * 2.0f;
            radius = std::max(2.0f, radius * (zoom * 0.03f + 0.5f));

            DrawCircleV(s, radius, WHITE);
        }

        DrawFPS(10, 10);
        DrawText(paused ? "PAUSED (SPACE to toggle)" : "RUNNING (SPACE to toggle)", 10, 30, 14,
                 WHITE);
        DrawText("Zoom: mouse wheel or UP/DOWN. Pan: WASD / arrows.", 10, 50, 14, WHITE);
        DrawText("Sim speed: +/- (keyboard). Reset view: R", 10, 70, 14, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
