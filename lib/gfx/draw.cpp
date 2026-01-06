
#include <raylib.h>

#include <cstdlib>
#include <string>

#include "base/type.hpp"
#include "gfx/draw.hpp"
#include "math/vec.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

template <FloatT Float>
void draw_ruler(const Camera<Float>& camera, const std::string& text) {
    I32               width   = 256;
    nbody::gfx::Point point_a = {(camera.screen_width / 2) - (width / 2),
                                 camera.screen_height - 32};
    nbody::gfx::Point point_b = {(camera.screen_width / 2) + (width / 2),
                                 camera.screen_height - 32};

    DrawRectangle(point_a.x, point_a.y, width, 2, WHITE);
    DrawRectangle(point_a.x, point_a.y - 16, 2, 32, WHITE);
    DrawRectangle(point_b.x, point_b.y - 16, 2, 32, WHITE);

    math::Vec2T<Float> world_a      = camera.screen_to_world(point_a);
    math::Vec2T<Float> world_b      = camera.screen_to_world(point_b);
    Float              distance     = world_a.distance(world_b);
    std::string        display_text = std::to_string(distance) + text;

    DrawText(display_text.c_str(), point_a.x + 48, point_a.y - 32, 24, WHITE);
}

template void draw_ruler(const Camera<F32>& camera, const std::string& text);
template void draw_ruler(const Camera<F64>& camera, const std::string& text);

}  // namespace nbody::gfx
