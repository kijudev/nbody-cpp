// ==============================================================================
// generator.hpp
// Distribution generators for positions, masses, and velocities in n-body
// simulations. Includes formulas for each generator function.
// ==============================================================================

#pragma once

#include <math.h>
#include <raylib.h>

#include <cmath>
#include <functional>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// Function type for generating a position distribution.
// Returns a vector of positions for n bodies within a given radius.
template <FloatT Float>
using GeneratePositionDistributionFn =
    std::function<std::vector<math::Vec2T<Float>>(USize n, Float radius)>;

// Function type for generating a mass distribution.
// Returns a vector of masses for n bodies within [min_mass, max_mass].
template <FloatT Float>
using GenerateMassDistributionFn =
    std::function<std::vector<Float>(USize n, Float min_mass, Float max_mass)>;

// Formula:
// x = random value in [-radius, radius]
// y = random value in [-radius, radius]
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_box(
    USize n, Float radius);

// Formula:
// X = random value in [0, 1]
// r = radius * sqrt(X)
// theta = random value in [0, 2*pi]
// x = r * cos(theta)
// y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_disk(
    USize n, Float radius);

// Formula:
// X = random value in [0, 1]
// r = radius * (1 / sqrt(X^(-2/3) - 1))
// theta = random value in [0, 2*pi]
// x = r * cos(theta)
// y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_plummer_model(
    USize n, Float radius);

// Formula:
// m = random value in [min_mass, max_mass]
// m ~ U(Mmin, Mmax)
template <FloatT Float>
std::vector<Float> generate_mass_distribution_uniform(USize n, Float min_mass,
                                                      Float max_mass);

// Formula:
// Salpeter IMF: dN/dm ~ m^(-2.35)
// m = ( (X * (max_mass^(1-2.35) - min_mass^(1-2.35)) + min_mass^(1-2.35) )
// )^(1/(1-2.35)) where X = random value in [0, 1]
template <FloatT Float>
std::vector<Float> generate_mass_distribution_salpeter_imf(USize n,
                                                           Float min_mass,
                                                           Float max_mass);

// Function type for generating a velocity distribution.
// Returns a vector of velocities for each body, given their positions, masses,
// the system center, central mass, and gravitational constant.
template <FloatT Float>
using GenerateVelocityDistributionFn =
    std::function<std::vector<math::Vec2T<Float>>(
        const std::vector<math::Vec2T<Float>>& positions,
        const std::vector<Float>& masses, math::Vec2T<Float> center,
        Float central_mass, Float g)>;

// Formula:
// v = (0, 0) for all bodies
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_velocity_distribution_zero(
    const std::vector<math::Vec2T<Float>>& positions,
    const std::vector<Float>& masses, math::Vec2T<Float> center,
    Float central_mass, Float g);

// Formula:
// For each body at position r (relative to center):
// v = sqrt(G * central_mass / |r|)
// Direction: perpendicular to r (tangential velocity)
template <typename Float>
std::vector<math::Vec2T<Float>> generate_velocity_distribution_circular(
    const std::vector<math::Vec2T<Float>>& positions,
    const std::vector<Float>& masses, math::Vec2T<Float> center,
    Float central_mass, Float g);

// Configuration struct for generating a full distribution of bodies.
// Specifies the number of bodies, mass/radius ranges, velocity center, central
// mass, and which generator functions to use for position, mass, and velocity.
template <FloatT Float>
struct GenerateDistributionConfig {
    USize              n               = 0;     // Number of bodies
    Float              min_mass        = 1.0;   // Minimum mass
    Float              max_mass        = 1.0;   // Maximum mass
    Float              radius          = 10.0;  // Distribution radius
    math::Vec2T<Float> velocity_center = {
        0.0, 0.0};             // Center for velocity distribution
    Float central_mass = 0.0;  // Central mass for velocity distribution

    GeneratePositionDistributionFn<Float> position_fn =
        generate_position_distribution_uniform_box<Float>;
    GenerateMassDistributionFn<Float> mass_fn =
        generate_mass_distribution_uniform<Float>;
    GenerateVelocityDistributionFn<Float> velocity_fn =
        generate_velocity_distribution_zero<Float>;
};

// Generates a vector of bodies using the provided distribution config.
// Applies the selected position, mass, and velocity generators to produce a
// full set of bodies.
template <FloatT Float>
std::vector<BodyT<Float>> generate_distribution(
    const GenerateDistributionConfig<Float>& config);

}  // namespace nbody::sim
