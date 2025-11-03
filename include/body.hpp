#pragma once

#include "raylib.h"

namespace sim {
class Body {
   public:
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    double mass;

    Body(Vector2 arg_pos, Vector2 arg_vel, double arg_mass);

    void update(double dt);
};
}  // namespace sim
