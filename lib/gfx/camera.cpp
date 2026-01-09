#include "gfx/camera.hpp"

#include <raylib.h>

#include <cmath>
#include <cstdlib>
#include <tuple>

#include "base/type.hpp"
#include "gfx/point.hpp"
#include "math/collision.hpp"

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
Camera<Float>::Vec2 Camera<Float>::world_to_screen_vec(const Vec2& world_pos) const {
    Vec2 screen_pos = world_pos.sub(pos).scale(zoom);

    return {
        static_cast<Float>(screen_pos.x + (screen_width / 2.0)),
        static_cast<Float>((screen_height / 2.0) - screen_pos.y),
    };
}

template <FloatT Float>
typename Camera<Float>::Vec2 Camera<Float>::screen_to_world(Point point) const {
    Float cx = static_cast<Float>(point.x) - (screen_width / 2.0);
    Float cy = static_cast<Float>(point.y) - (screen_height / 2.0);

    return {
        pos.x + (cx / zoom),
        pos.y - (cy / zoom),
    };
}

template <FloatT Float>
std::tuple<typename Camera<Float>::Vec2, typename Camera<Float>::Vec2>
Camera<Float>::viewport_extent_world() const {
    return std::make_tuple(screen_to_world({0, 0}), screen_to_world({screen_width, screen_height}));
}

template <FloatT Float>
bool Camera<Float>::is_pos_in_viewport(const Vec2& pos) const {
    Vec2 w0      = screen_to_world({0, 0});
    Vec2 wscreen = screen_to_world({screen_width, screen_height});

    return math::check_collision2_rect_point(w0.x, w0.y, std::abs(wscreen.x - w0.x),
                                             std::abs(wscreen.y - w0.y), pos.x, pos.y);
}

template <FloatT Float>
bool Camera<Float>::is_circle_in_viewport(const Vec2& center, Float radius) const {
    Vec2 w0      = screen_to_world({0, 0});
    Vec2 wscreen = screen_to_world({screen_width, screen_height});

    return math::check_collision2_rect_circle(w0.x, w0.y, std::abs(wscreen.x - w0.x),
                                              std::abs(wscreen.y - w0.y), center.x, center.y,
                                              radius);
}

template <FloatT Float>
void Camera<Float>::handle_controls(Float dt) {
    // NOTE: RIGHT / LEFT
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        pos.x += movement_speed * dt / zoom;
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        pos.x -= movement_speed * dt / zoom;
    }

    // NOTE: UP / DOWN
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        pos.y += movement_speed * dt / zoom;
    } else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        pos.y -= movement_speed * dt / zoom;
    }

    zoom =
        static_cast<Float>(std::expf(std::logf(static_cast<F32>(zoom)) +
                                     static_cast<F32>(GetMouseWheelMove() * scaling_speed * dt)));
}

template struct Camera<F32>;
template struct Camera<F64>;
}  // namespace nbody::gfx
