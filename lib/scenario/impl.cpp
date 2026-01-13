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

}  // namespace nbody::scenario::impl
