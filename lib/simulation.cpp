#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <random>
#include <utility>

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
                    Vector2Scale(r, mj / (std::max(magsq * mag, 1.0f)));

                bodies[i].acc = Vector2Add(bodies[i].acc, ai);
            }
        }
    }
}

void Simulation::place_body(Body body) { bodies.push_back(body); }

void Simulation::place_random_body() {
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist_pos(-1000.0, 1000.0);
    std::uniform_real_distribution<float> dist_vel(-100.0, 100.0);
    std::uniform_real_distribution<float> dist_mass(1.0, 10000.0);

    Body body((Vector2){dist_pos(gen), dist_pos(gen)},
              (Vector2){dist_vel(gen), dist_vel(gen)}, dist_mass(gen));

    place_body(body);
}
}  // namespace sim
