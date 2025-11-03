#include "body.hpp"
#include "raymath.h"

namespace sim {
Body::Body(Vector2 arg_pos, Vector2 arg_vel, double arg_mass)
    : pos(arg_pos), vel(arg_vel), acc({0, 0}), mass(arg_mass) {}
}  // namespace sim
