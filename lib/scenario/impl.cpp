#include "scenario/impl.hpp"

#include <raygui.h>
#include <raylib.h>

#include <format>
#include <optional>
#include <string>

#include "base/type.hpp"
#include "sim/const.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

template <FloatT Float>
std::string format_time(Float factor, Float year, Float day, Float hour,
                        Float minute) {
    if (factor >= year) {
        return std::format("{:.2f} years/sec", factor / year);
    } else if (factor >= day) {
        return std::format("{:.2f} days/sec", factor / day);
    } else if (factor >= hour) {
        return std::format("{:.2f} hours/sec", factor / hour);
    } else if (factor >= minute) {
        return std::format("{:.2f} mins/sec", factor / minute);
    } else {
        return std::format("{:.2f} sec/sec", factor);
    }
}

template std::string format_time(F32, F32, F32, F32, F32);
template std::string format_time(F64, F64, F64, F64, F64);

template <FloatT Float>
std::optional<USize> get_body_at_mouse_position_au(
    gfx::Camera<Float>                                      camera,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies) {
    gfx::Point         mouse_pos{GetMouseX(), GetMouseY()};
    math::Vec2T<Float> world_pos = camera.screen_to_world(mouse_pos);

    Float min_distance = std::numeric_limits<Float>::max();
    USize index        = 0;

    for (USize i = 0; i < bodies.size(); ++i) {
        Float dist = bodies[i].pos.distance(world_pos);

        if (dist < min_distance) {
            min_distance = dist;
            index        = i;
        }
    }

    Float threshold = sim::scale_au::DISTANCE_AU * 50.0 / camera.zoom;

    if (min_distance < threshold) {
        return index;
    }

    return std::nullopt;
}

template std::optional<USize> get_body_at_mouse_position_au(
    gfx::Camera<F32>, std::span<const sim::BodyT<F32>, std::dynamic_extent>);
template std::optional<USize> get_body_at_mouse_position_au(
    gfx::Camera<F64>, std::span<const sim::BodyT<F64>, std::dynamic_extent>);

template <FloatT Float>
void handle_slingshot_input(SlingshotState<Float>&    state,
                            const gfx::Camera<Float>& camera) {
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state.is_active = true;

        Vector2 mouse_pos = GetMousePosition();
        state.start_pos   = camera.screen_to_world(gfx::Point{
            static_cast<I32>(mouse_pos.x), static_cast<I32>(mouse_pos.y)});
        state.current_pos = state.start_pos;
    }

    if (state.is_active && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse_pos = GetMousePosition();
        state.current_pos = camera.screen_to_world(gfx::Point{
            static_cast<I32>(mouse_pos.x), static_cast<I32>(mouse_pos.y)});
    }

    if (state.is_active && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        state.is_active = false;
    }
}

template void handle_slingshot_input(SlingshotState<F32>&,
                                     const gfx::Camera<F32>&);
template void handle_slingshot_input(SlingshotState<F64>&,
                                     const gfx::Camera<F64>&);

template <FloatT Float>
void draw_slingshot(const SlingshotState<Float>& state,
                    const gfx::Camera<Float>& camera, Float scale_factor) {
    if (!state.is_active) {
        return;
    }

    using Vec2 = math::Vec2T<Float>;

    Float radius = state.radius_scale / camera.zoom;

    gfx::Point center_screen  = camera.world_to_screen(state.start_pos);
    gfx::Point current_screen = camera.world_to_screen(state.current_pos);

    Vec2       edge_pos = Vec2{state.start_pos.x + radius, state.start_pos.y};
    gfx::Point edge_screen = camera.world_to_screen(edge_pos);

    Float screen_radius =
        std::abs(edge_screen.x - center_screen.x) * scale_factor;

    DrawCircle(center_screen.x, center_screen.y, screen_radius,
               ColorAlpha(YELLOW, 0.6f));
    DrawCircleLines(center_screen.x, center_screen.y, screen_radius, GOLD);

    DrawLineEx(Vector2{static_cast<F32>(center_screen.x),
                       static_cast<F32>(center_screen.y)},
               Vector2{static_cast<F32>(current_screen.x),
                       static_cast<F32>(current_screen.y)},
               3.0f, ORANGE);

    Vec2  direction   = state.current_pos.sub(state.start_pos);
    Float line_length = direction.length();

    if (line_length > 0.01) {
        DrawCircle(current_screen.x, current_screen.y, 5.0f, RED);
    }

    Vec2  drag_vector        = state.current_pos.sub(state.start_pos);
    Float predicted_velocity = drag_vector.length() * state.velocity_scale;
    Float mass               = state.base_mass / (camera.zoom * camera.zoom);

    std::string info = std::format("Mass: {:.2e} kg | Velocity: {:.2f} AU/yr",
                                   mass, predicted_velocity);
    DrawText(info.c_str(), 10, 10, 20, RAYWHITE);
}

template void draw_slingshot(const SlingshotState<F32>&,
                             const gfx::Camera<F32>&, F32);
template void draw_slingshot(const SlingshotState<F64>&,
                             const gfx::Camera<F64>&, F64);

template <FloatT Float>
sim::BodyT<Float> create_slingshot_body(const SlingshotState<Float>& state,
                                        const gfx::Camera<Float>&    camera) {
    using Vec2 = math::Vec2T<Float>;

    Vec2 drag_vector = state.current_pos.sub(state.start_pos);
    Vec2 velocity    = drag_vector.scale(-state.velocity_scale).scale(1 / (2 * 42.0));
    ;
    Float mass = 10.0 * state.base_mass / (camera.zoom * camera.zoom);

    return sim::BodyT<Float>{
        .pos  = state.start_pos,
        .vel  = velocity,
        .acc  = Vec2::make_zero(),
        .mass = mass,
    };
}

template sim::BodyT<F32> create_slingshot_body(const SlingshotState<F32>&,
                                               const gfx::Camera<F32>&);
template sim::BodyT<F64> create_slingshot_body(const SlingshotState<F64>&,
                                               const gfx::Camera<F64>&);

}  // namespace nbody::scenario::impl
