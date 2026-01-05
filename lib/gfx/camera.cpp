#include <raylib.h>

#include <cmath>

#include "base/type.hpp"
#include "gfx/camera.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

template <FloatT Float>
Point Camera<Float>::world_to_screen(const Vec2& world_pos) const {
    Vec2 translate = world_pos.sub(pos).scale(zoom);

    return {
        .x = static_cast<I32>(translate.x + (screen_width / 2.0)),
        .y = static_cast<I32>((screen_height / 2.0) - translate.y),
    };
}

template <FloatT Float>
typename Camera<Float>::Vec2 Camera<Float>::screen_to_world(Point point) const {
    Float centered_x = static_cast<Float>(point.x) - (screen_width / 2.0);
    Float centered_y = static_cast<Float>(point.y) - (screen_height / 2.0);

    return {
        .x = pos.x + (centered_x / zoom),
        .y = pos.y - (centered_y / zoom),
    };
}

template <FloatT Float>
bool Camera<Float>::is_pos_in_viewport(const Vec2& pos) const {
    (void)pos;
    return false;
}

template <FloatT Float>
bool Camera<Float>::is_circle_in_viewport(const Vec2& center, Float radius) const {
    (void)center;
    (void)radius;
    return false;
}

template <FloatT Float>
void Camera<Float>::handle_controls(Float dt) {
    // RIGHT / LEFT
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        pos.x += movement_speed * dt;
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        pos.x -= movement_speed * dt;
    }

    // UP / DOWN
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        pos.y += movement_speed * dt;
    } else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        pos.y -= movement_speed * dt;
    }

    // NOTE: Uses log scaling.
    // TODO: Create a expf and logf impl for FloatT.
    zoom = static_cast<Float>(std::expf(std::logf(static_cast<F32>(zoom)) +
                                        static_cast<F32>(GetMouseWheelMove() * movement_speed)));
}
}  // namespace nbody::gfx
