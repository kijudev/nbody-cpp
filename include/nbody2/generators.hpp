#include <math.h>
#include <raymath.h>

#include <cmath>
#include <functional>
#include <random>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "nbody2/type.hpp"

namespace nbody2 {

template <FloatT Float>
using GeneratePositionDistributionFn =
    std::function<std::vector<math::Vec2T<Float>>(USize n, Float radius)>;

template <FloatT Float>
using GenerateMassDistributionFn =
    std::function<std::vector<Float>(USize n, Float min_mass, Float max_mass)>;

// NOTE: This function generates a uniform distribution of positions within a box of given radius
// (2*r is the side length). The center of the box is at the origin (0, 0).
// x ~ U(-r, r)
// y ~ U(-r, r)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_box(USize n, Float radius) {
    std::vector<math::Vec2T<Float>> positions;
    positions.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(-radius, radius);

    for (USize i = 0; i < n; ++i) {
        Float x = uniform_dist(random_engine);
        Float y = uniform_dist(random_engine);
        positions.push_back({x, y});
    }

    return positions;
}

// NOTE: This function generates a uniform distribution of positions within a disk of given radius.
// The center of the disk is at the origin (0, 0).
// X = rand(0, 1)
// r = Rmax * sqrt(X)
// theta = rand(0, 2*pi)
// NOTE: Convert from polar to Cartesian coordinates.
// x = r * cos(theta)
// y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_disk(USize n, Float radius) {
    std::vector<math::Vec2T<Float>> positions;
    positions.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(0.0, 1.0);

    for (USize i = 0; i < n; ++i) {
        Float X     = uniform_dist(random_engine);
        Float r     = std::sqrt(X) * radius;
        Float theta = uniform_dist(random_engine) * 2.0 * static_cast<Float>(PI);
        Float x     = r * std::cos(theta);
        Float y     = r * std::sin(theta);
        positions.push_back({x, y});
    }

    return positions;
}

// NOTE: Generate position distribution using Plummer model. The center of the Globular Cluster is
// at the origin (0, 0).
// X = rand(0, 1)
// r = Rmax * (1 / sqrt(X ** (-2/3) - 1))
// theta = rand(0, 2pi)
// NOTE: Convert from polar to Cartesian coordinates.
// x = r * cos(theta)
// y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_plummer_model(USize n,
                                                                             Float radius) {
    std::vector<math::Vec2T<Float>> positions;
    positions.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(0.0, 1.0);

    for (USize i = 0; i < n; ++i) {
        Float X     = uniform_dist(random_engine);
        Float r     = radius * (1.0 / std::sqrt(std::pow(X, -2.0 / 3.0) - 1.0));
        Float theta = uniform_dist(random_engine) * 2.0 * static_cast<Float>(PI);
        Float x     = r * std::cos(theta);
        Float y     = r * std::sin(theta);
        positions.push_back({x, y});
    }

    return positions;
}

// NOTE: Generate a uniform mass distribution.
// m ~ U(Mmin, Mmax)
template <FloatT Float>
std::vector<Float> generate_mass_distribution_uniform(USize n, Float min_mass, Float max_mass) {
    std::vector<Float> masses;
    masses.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(min_mass, max_mass);

    for (USize i = 0; i < n; ++i) {
        masses.push_back(uniform_dist(random_engine));
    }

    return masses;
}

// NOTE: Generate a mass distribution following the Salpeter imf.
template <FloatT Float>
std::vector<Float> generate_mass_distribution_salpeter_imf(USize n, Float min_mass,
                                                           Float max_mass) {
    constexpr Float ALPHA = 2.35;

    Float exponent = 1.0 - ALPHA;
    Float term_a   = std::pow(max_mass, exponent);
    Float term_b   = std::pow(min_mass, exponent);
    Float diff     = term_a - term_b;
    Float inv_exp  = 1.0 / exponent;

    std::vector<Float> masses;
    masses.reserve(n);

    static std::mt19937 random_engine(std::random_device{}());

    std::uniform_real_distribution<Float> uniform_dist(0.0, 1.0);

    for (std::size_t i = 0; i < n; ++i) {
        Float X    = uniform_dist(random_engine);
        Float base = X * diff + term_b;
        Float mass = std::pow(base, inv_exp);
        masses.push_back(mass);
    }

    return masses;
}

template <FloatT Float>
struct GenerateDistributionConfig {
    USize                                 n        = 0;
    Float                                 min_mass = 1.0;
    Float                                 max_mass = 1.0;
    Float                                 radius   = 10.0;
    GeneratePositionDistributionFn<Float> position_fn =
        generate_position_distribution_uniform_box<Float>;
    GenerateMassDistributionFn<Float> mass_fn = generate_mass_distribution_uniform<Float>;
};

template <FloatT Float>
std::vector<BodyT<Float>> generate_distribution(const GenerateDistributionConfig<Float>& config) {
    std::vector<math::Vec2T<Float>> positions = config.position_fn(config.n, config.radius);
    std::vector<Float> masses = config.mass_fn(config.n, config.min_mass, config.max_mass);

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(config.n);

    for (USize i = 0; i < config.n; ++i) {
        bodies.push_back(BodyT<Float>{
            .pm = {.pos = positions[i], .mass = masses[i]}
        });
    }

    return bodies;
}

}  // namespace nbody2
