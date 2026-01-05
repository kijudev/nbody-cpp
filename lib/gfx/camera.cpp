#include "gfx/camera.hpp"

#include "base/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

template <FloatT Float>
Point Camera<Float>::world_to_screen(Vec2 world_pos) const {
    Vec2 translate = world_pos.sub(pos).scale(zoom);

    return {
        .x = static_cast<I32>(translate.x + (screen_width / 2.0)),
        .y = static_cast<I32>((screen_height / 2.0) - translate.y),
    };
}

template <FloatT Float>
typename Camera<Float>::Vec2 Camera<Float>::screen_to_world(Point point) {
    Float centered_x = static_cast<Float>(point.x) - (screen_width / 2.0);
    Float centered_y = static_cast<Float>(point.y) - (screen_height / 2.0);

    return {
        .x = pos.x + (centered_x / zoom),
        .y = pos.y - (centered_y / zoom),
    };
}
}  // namespace nbody::gfx
