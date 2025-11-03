#include <raylib.h>
#include <raymath.h>

#include <cstddef>

#include "simulation.hpp"

namespace sim {
void Simulation::update() {
    for (size_t i = 0; i < bodies.size(); ++i) {
        Vector2 pi = bodies[i].pos;

        for (size_t j = 0; j < bodies.size(); ++j) {
            if (j != i) {
                Vector2 pj = bodies[j].pos;
                double mj = bodies[j].mass;

                Vector2 r = Vector2Subtract(pi, pj);
                double magsq = Vector2DistanceSqr(pi, pj);
                double mag = Vector2Distance(pi, pj);

                Vector2 ai = Vector2Scale(r, mj / (magsq * mag));

                bodies[i].acc.x += ai.x;
                bodies[i].acc.y += ai.y;
            }
        }
    }
}
}  // namespace sim
