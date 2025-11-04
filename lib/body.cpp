
#include <iostream>

#include "body.hpp"
#include "raylib.h"

namespace sim {
Body::Body(Vector2 arg_pos, Vector2 arg_vel, double arg_mass)
    : pos(arg_pos), vel(arg_vel), acc({0, 0}), mass(arg_mass) {}

void Body::update(double dt) {
    vel.x += acc.x * dt;
    vel.y += acc.y * dt;
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    acc.x = 0;
    acc.y = 0;
}
}  // namespace sim
