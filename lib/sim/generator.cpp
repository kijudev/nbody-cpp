#include "sim/generator.hpp"

#include <math.h>
#include <raymath.h>

#include <cmath>
#include <random>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "sim/const.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_box(
    USize n, Float radius) {
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

template std::vector<math::Vec2F32> generate_position_distribution_uniform_box(
    USize n, F32 radius);
template std::vector<math::Vec2F64> generate_position_distribution_uniform_box(
    USize n, F64 radius);

template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_disk(
    USize n, Float radius) {
    std::vector<math::Vec2T<Float>> positions;
    positions.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(0.0, 1.0);

    for (USize i = 0; i < n; ++i) {
        Float X = uniform_dist(random_engine);
        Float r = std::sqrt(X) * radius;
        Float theta =
            uniform_dist(random_engine) * 2.0 * static_cast<Float>(PI);
        Float x = r * std::cos(theta);
        Float y = r * std::sin(theta);
        positions.push_back({x, y});
    }

    return positions;
}

template std::vector<math::Vec2F32> generate_position_distribution_uniform_disk(
    USize n, F32 radius);
template std::vector<math::Vec2F64> generate_position_distribution_uniform_disk(
    USize n, F64 radius);

// NOTE: Generate position distribution using Plummer model. The center of the
// Globular Cluster is at the origin (0, 0). X = rand(0, 1) r = Rmax * (1 /
// sqrt(X ** (-2/3) - 1)) theta = rand(0, 2pi) NOTE: Convert from polar to
// Cartesian coordinates. x = r * cos(theta) y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_plummer_model(
    USize n, Float radius) {
    std::vector<math::Vec2T<Float>> positions;
    positions.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(0.0, 1.0);

    for (USize i = 0; i < n; ++i) {
        Float X = uniform_dist(random_engine);
        Float r = radius * (1.0 / std::sqrt(std::pow(X, -2.0 / 3.0) - 1.0));
        Float theta =
            uniform_dist(random_engine) * 2.0 * static_cast<Float>(PI);
        Float x = r * std::cos(theta);
        Float y = r * std::sin(theta);
        positions.push_back({x, y});
    }

    return positions;
}

template std::vector<math::Vec2F32>
generate_position_distribution_plummer_model(USize n, F32 radius);
template std::vector<math::Vec2F64>
generate_position_distribution_plummer_model(USize n, F64 radius);

// NOTE: Generate a uniform mass distribution.
// m ~ U(Mmin, Mmax)
template <FloatT Float>
std::vector<Float> generate_mass_distribution_uniform(USize n, Float min_mass,
                                                      Float max_mass) {
    std::vector<Float> masses;
    masses.reserve(n);

    std::default_random_engine            random_engine;
    std::uniform_real_distribution<Float> uniform_dist(min_mass, max_mass);

    for (USize i = 0; i < n; ++i) {
        masses.push_back(uniform_dist(random_engine));
    }

    return masses;
}

template std::vector<F32> generate_mass_distribution_uniform(USize n,
                                                             F32   min_mass,
                                                             F32   max_mass);
template std::vector<F64> generate_mass_distribution_uniform(USize n,
                                                             F64   min_mass,
                                                             F64   max_mass);

// NOTE: Generate a mass distribution following the Salpeter imf.
template <FloatT Float>
std::vector<Float> generate_mass_distribution_salpeter_imf(USize n,
                                                           Float min_mass,
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

template std::vector<F32> generate_mass_distribution_salpeter_imf(USize n,
                                                                  F32 min_mass,
                                                                  F32 max_mass);
template std::vector<F64> generate_mass_distribution_salpeter_imf(USize n,
                                                                  F64 min_mass,
                                                                  F64 max_mass);

template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_velocity_distribution_zero(
    const std::vector<math::Vec2T<Float>>& positions,
    const std::vector<Float>& masses, math::Vec2T<Float> center,
    Float central_mass, Float g) {
    (void)center;
    (void)central_mass;
    (void)g;
    (void)positions;
    (void)masses;

    return std::vector<math::Vec2T<Float>>(positions.size(),
                                           math::Vec2T<Float>{0.0, 0.0});
}

template std::vector<math::Vec2T<F32>> generate_velocity_distribution_zero(
    const std::vector<math::Vec2T<F32>>& positions,
    const std::vector<F32>& masses, math::Vec2T<F32> center, F32 central_mass,
    F32 g);
template std::vector<math::Vec2T<F64>> generate_velocity_distribution_zero(
    const std::vector<math::Vec2T<F64>>& positions,
    const std::vector<F64>& masses, math::Vec2T<F64> center, F64 central_mass,
    F64 g);

template <typename Float>
std::vector<nbody::math::Vec2T<Float>> generate_velocity_distribution_circular(
    const std::vector<nbody::math::Vec2T<Float>>& positions,
    const std::vector<Float>& masses, nbody::math::Vec2T<Float> center,
    Float central_mass, Float g) {
    (void)masses;

    std::vector<nbody::math::Vec2T<Float>> velocities;
    velocities.reserve(positions.size());

    for (const auto& pos : positions) {
        nbody::math::Vec2T<Float> r    = pos.sub(center);
        Float                     dist = r.length();

        if (dist == 0.0) {
            velocities.push_back({0.0, 0.0});
            continue;
        }

        Float                     v     = std::sqrt(g * central_mass / dist);
        nbody::math::Vec2T<Float> v_dir = {-r.y / dist, r.x / dist};

        velocities.push_back(v_dir.scale(v));
    }
    return velocities;
}

template std::vector<nbody::math::Vec2T<F32>>
generate_velocity_distribution_circular(
    const std::vector<math::Vec2T<F32>>& positions,
    const std::vector<F32>& masses, math::Vec2T<F32> center, F32 central_mass,
    F32 g);
template std::vector<nbody::math::Vec2T<F64>>
generate_velocity_distribution_circular(
    const std::vector<math::Vec2T<F64>>& positions,
    const std::vector<F64>& masses, math::Vec2T<F64> center, F64 central_mass,
    F64 g);

template <FloatT Float>
std::vector<BodyT<Float>> generate_distribution(
    const GenerateDistributionConfig<Float>& config) {
    std::vector<math::Vec2T<Float>> positions =
        config.position_fn(config.n, config.radius);
    std::vector<Float> masses =
        config.mass_fn(config.n, config.min_mass, config.max_mass);

    std::vector<math::Vec2T<Float>> velocities;

    if (config.velocity_fn) {
        velocities =
            config.velocity_fn(positions, masses, config.velocity_center,
                               config.central_mass, sim::scale_toy::G);
    } else {
        velocities = std::vector<math::Vec2T<Float>>(
            config.n, math::Vec2T<Float>{0.0, 0.0});
    }

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(config.n);

    for (USize i = 0; i < config.n; ++i) {
        bodies.push_back(BodyT<Float>{
            .pos  = positions[i],
            .vel  = velocities[i],
            .mass = masses[i],
        });
    }

    return bodies;
}

template struct GenerateDistributionConfig<F32>;
template struct GenerateDistributionConfig<F64>;

template std::vector<BodyF32> generate_distribution(
    const GenerateDistributionConfig<F32>& config);
template std::vector<BodyF64> generate_distribution(
    const GenerateDistributionConfig<F64>& config);

}  // namespace nbody::sim
