#pragma once

#include "gfx/window.hpp"

namespace nbody::scenario {
class ScenarioInterface {
   public:
    virtual ~ScenarioInterface() = default;

    virtual void init(const gfx::Window& window) = 0;
    virtual void step(const gfx::Window& window) = 0;
};
}  // namespace nbody::scenario
