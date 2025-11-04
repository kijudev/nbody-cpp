#pragma once

#include <vector>

#include "body.hpp"

namespace sim {
class Simulation {
   public:
    std::vector<Body> bodies;

    void update();
    void place_body(Body body);
    void place_random_body();
};
}  // namespace sim
