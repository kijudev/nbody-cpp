// ==============================================================================
// type.hpp
// Common types and interfaces for n-body simulations.
// Defines the Body type, simulation interface, and integration function types.
// ==============================================================================
#pragma once

#include <functional>
#include <span>

#include "base/type.hpp"
#include "math/vec.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// ==============================================================================
// Body Type
// ==============================================================================
// Represents a single body in the simulation (position, velocity, acceleration,
// mass).
template <FloatT Float>
struct BodyT {
    math::Vec2T<Float> pos{0.0, 0.0};  // Position in 2D space
    math::Vec2T<Float> vel{0.0, 0.0};  // Velocity in 2D space
    math::Vec2T<Float> acc{0.0, 0.0};  // Acceleration in 2D space
    Float              mass{0.0};      // Mass of the body
};

using BodyF32 = BodyT<F32>;
using BodyF64 = BodyT<F64>;  // Convenient aliases for float/double bodies

// ==============================================================================
// Integration Function Type
// ==============================================================================
// Function type for integrating a single body over a timestep.
template <FloatT Float>
using IntegrateBodyFnT = std::function<void(BodyT<Float>& body, Float dt)>;

// ==============================================================================
// Simulation Interface
// ==============================================================================
// Base interface for all simulation implementations.
// Provides methods for stepping the simulation, accessing bodies, and inserting
// new bodies.
template <FloatT Float>
class SimInterface {
   public:
    virtual ~SimInterface() = default;

    // Advance the simulation by one timestep (dt).
    virtual void step(Float dt) = 0;

    // Return a span of all bodies in the simulation (read-only).
    virtual std::span<const BodyT<Float>, std::dynamic_extent> bodies()
        const = 0;

    // Insert a new body into the simulation.
    virtual void insert_body(BodyT<Float>&& body) = 0;
};
}  // namespace nbody::sim
