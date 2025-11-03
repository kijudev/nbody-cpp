#pragma once

#include <vector>

#include "body.hpp"

namespace sim {
class Simulation {
   public:
    std::vector<Body> bodies;

    void update();
};
}  // namespace sim
