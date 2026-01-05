#include <tuple>

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

struct Point {
    I32 x{0}, y{0};
};

template <FloatT Float>
struct Camera {
    using Vec2 = math::Vec2T<Float>;

    I32   screen_width{800};
    I32   screen_height{600};
    Vec2  pos{0.0, 0.0};
    Float zoom{1.0};
    Float movement_speed{1.0};

    Point world_to_screen(const Vec2& world_pos) const;
    Vec2  screen_to_world(Point point) const;

    std::tuple<Vec2, Vec2> viewport_extent_world() const;
    bool                   is_pos_in_viewport(const Vec2& pos) const;
    bool                   is_circle_in_viewport(const Vec2& center, Float radius) const;

    // NOTE: Handles basic camera controls:
    // - UP, DOWN, RIGHT, LEFT (arrows, WSAD).
    // - Scolling.
    // WARNING: This method need to be called on every game loop interation.
    void handle_controls(Float dt);
};

}  // namespace nbody::gfx
