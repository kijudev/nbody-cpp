#include <math.h>
#include <raymath.h>

#include <cmath>
#include <functional>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

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
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_box(USize n, Float radius);

// NOTE: This function generates a uniform distribution of positions within a disk of given radius.
// The center of the disk is at the origin (0, 0).
// X = rand(0, 1)
// r = Rmax * sqrt(X)
// theta = rand(0, 2*pi)
// NOTE: Convert from polar to Cartesian coordinates.
// x = r * cos(theta)
// y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_uniform_disk(USize n, Float radius);

// NOTE: Generate position distribution using Plummer model. The center of the Globular Cluster is
// at the origin (0, 0).
// X = rand(0, 1)
// r = Rmax * (1 / sqrt(X ** (-2/3) - 1))
// theta = rand(0, 2pi)
// NOTE: Convert from polar to Cartesian coordinates.
// x = r * cos(theta)
// y = r * sin(theta)
template <FloatT Float>
std::vector<math::Vec2T<Float>> generate_position_distribution_plummer_model(USize n, Float radius);

// NOTE: Generate a uniform mass distribution.
// m ~ U(Mmin, Mmax)
template <FloatT Float>
std::vector<Float> generate_mass_distribution_uniform(USize n, Float min_mass, Float max_mass);

// NOTE: Generate a mass distribution following the Salpeter imf.
template <FloatT Float>
std::vector<Float> generate_mass_distribution_salpeter_imf(USize n, Float min_mass, Float max_mass);

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
std::vector<BodyT<Float>> generate_distribution(const GenerateDistributionConfig<Float>& config);

}  // namespace nbody::sim
