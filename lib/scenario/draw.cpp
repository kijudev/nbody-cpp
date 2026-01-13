#include <raylib.h>

#include <array>
#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "scenario/draw.hpp"
#include "sim/type.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

template <FloatT Float>
void draw_ruler_au(const gfx::Camera<Float>& camera, I32 ruler_padding,
                   I32 ruler_height, Float max_screen_fraction) {
    constexpr std::array<Float, 15> lengths = {
        0.001, 0.01, 0.1,  0.5,  1,     5,     10,    50,
        100,   500,  1000, 5000, 10000, 50000, 100000};

    Float world_len = lengths[0];
    I32   ruler_len = 0;

    for (Float len : lengths) {
        I32 candidate = len * camera.zoom;

        if (candidate < camera.screen_width * max_screen_fraction) {
            world_len = len;
            ruler_len = candidate;
        } else {
            break;
        }
    }

    I32 ruler_x = camera.screen_width - ruler_len - ruler_padding;
    I32 ruler_y = camera.screen_height - ruler_height - ruler_padding;

    DrawRectangle(ruler_x - 12, ruler_y - 12, ruler_len + 24, ruler_height + 12,
                  ColorAlpha(BLACK, 1.0f));

    F32 rx = ruler_x, ry = ruler_y;
    F32 rw = ruler_len, rh = ruler_height;

    DrawLineEx(Vector2{rx, ry + rh / 2}, Vector2{rx + rw, ry + rh / 2}, 2,
               WHITE);

    DrawLineEx(Vector2{rx, ry + 8}, Vector2{rx, ry + rh - 8}, 8, WHITE);

    DrawLineEx(Vector2{rx + rw, ry + 8}, Vector2{rx + rw, ry + rh - 8}, 2,
               WHITE);

    std::string label       = std::format("{:.3g} AU", world_len);
    I32         label_width = MeasureText(label.c_str(), 16);

    DrawText(label.c_str(), ruler_x + (ruler_len - label_width) / 2,
             ruler_y + ruler_height / 2 + 12, 24, WHITE);
}

template void draw_ruler_au(const gfx::Camera<F32>&, I32, I32, F32);
template void draw_ruler_au(const gfx::Camera<F64>&, I32, I32, F64);

template <FloatT Float>
void draw_bodies_monocolor(
    const gfx::Camera<Float> camera, Float scale_factor,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies,
    Color                                                   color) {
    for (const sim::BodyT<Float>& body : bodies) {
        const math::Vec2T<Float> center = camera.world_to_screen_vec(body.pos);
        const math::Vec2T<Float> edge   = camera.world_to_screen_vec(
            math::Vec2T<Float>{body.pos.x + std::cbrt(body.mass), body.pos.y});

        const Float radius = center.distance(edge) * scale_factor;

        if (radius > 0.5 && center.x + radius >= 0 &&
            center.x - radius <= camera.screen_width &&
            center.y + radius >= 0 &&
            center.y - radius <= camera.screen_height) {
            DrawCircleV(center.as_raylib_vector(), radius, color);
        }
    }
}

template void draw_bodies_monocolor(
    const gfx::Camera<F32>, F32,
    std::span<const sim::BodyT<F32>, std::dynamic_extent>, Color);
template void draw_bodies_monocolor(
    const gfx::Camera<F64>, F64,
    std::span<const sim::BodyT<F64>, std::dynamic_extent>, Color);

void draw_labels_vertical(const gfx::Box&                box,
                          const std::vector<std::string> labels, I32 font_size,
                          I32 gap, Color color) {
    for (USize i = 0; i < labels.size(); ++i) {
        I32 x = box.x;
        I32 y = box.y + (i * font_size);

        if (i > 0) {
            y += i * gap;
        }

        DrawText(labels[i].c_str(), x, y, font_size, color);
    }
}

template <FloatT Float>
void draw_cross_center(const gfx::Camera<Float>& camera, I32 size,
                       I32 thickness, Color color) {
    I32 cx = camera.screen_width / 2;
    I32 cy = camera.screen_height / 2;

    DrawRectangle(cx - size, cy - thickness / 2, 2 * size, thickness, color);
    DrawRectangle(cx - thickness / 2, cy - size, thickness, 2 * size, color);
}

template void draw_cross_center(const gfx::Camera<F32>&, I32, I32, Color);
template void draw_cross_center(const gfx::Camera<F64>&, I32, I32, Color);
}  // namespace nbody::scenario::impl
