#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cstddef>

#include "simulation.hpp"

namespace sim {
void Simulation::update() {
    for (size_t i = 0; i < bodies.size(); ++i) {
        Vector2 pi = bodies[i].pos;

        for (size_t j = 0; j < bodies.size(); ++j) {
            if (j != i) {
                Vector2 pj = bodies[j].pos;
                float mj = bodies[j].mass;

                Vector2 r = Vector2Subtract(pj, pi);
                float magsq = Vector2DistanceSqr(pj, pi);
                float mag = Vector2Distance(pj, pi);

                Vector2 ai =
                    Vector2Scale(r, mj / (std::max(magsq, 1.0f) * mag));
                bodies[i].acc.x += ai.x;
                bodies[i].acc.y += ai.y;
            }
        }
    }
}
}  // namespace sim
