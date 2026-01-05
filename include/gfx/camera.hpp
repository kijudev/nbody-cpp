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

    Point world_to_screen(math::Vec2T<Float> world_pos) const;
    Vec2  screen_to_world(Point point);
};

}  // namespace nbody::gfx
