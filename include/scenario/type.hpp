// ==============================================================================
// type.hpp
// Scenario interface base class for all simulation scenarios.
// Provides a common interface for initialization and simulation stepping.
// ==============================================================================

#pragma once

#include "gfx/window.hpp"

namespace nbody::scenario {

// ==============================================================================
// Scenario Interface Base Class
// ==============================================================================
// All scenario classes must inherit from this interface and implement
// initialization and simulation step methods.
class ScenarioInterface {
   public:
    virtual ~ScenarioInterface() = default;

    // Initialize the scenario with the given window properties.
    virtual void init(const gfx::Window& window) = 0;

    // Advance the scenario simulation by one step/frame.
    virtual void step(const gfx::Window& window) = 0;
};

}  // namespace nbody::scenario
