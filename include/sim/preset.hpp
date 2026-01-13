#pragma once

#include <vector>

#include "base/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/const.hpp"
#include "sim/direct.hpp"
#include "sim/generator.hpp"
#include "sim/type.hpp"

namespace nbody::sim::preset {
using namespace nbody::base::type;

namespace body {

template <FloatT Float>
std::vector<BodyT<Float>> two_body_circular(Float central_mass = 1.0, Float orbiter_mass = 1e-6,
                                            Float orbital_radius = 1.0, Float g = scale_au::G);

template <FloatT Float>
std::vector<BodyT<Float>> uniform_box(USize n, Float radius = 10.0, Float min_mass = 1.0,
                                      Float max_mass = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> uniform_disk(USize n, Float radius = 10.0, Float min_mass = 1.0,
                                       Float max_mass = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> plummer(USize n, Float radius = 10.0, Float min_mass = 1.0,
                                  Float max_mass = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> plummer_salpeter(USize n, Float radius = 10.0,
                                           Float min_mass = scale_au::MASS_HYGIEA,
                                           Float max_mass = scale_au::MASS_SOL * 10.0);
template <FloatT Float>
std::vector<BodyT<Float>> grid(USize nx, USize ny, Float spacing = 1.0, Float mass = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> ring(USize n, Float radius = 10.0, Float mass = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> solar_system_like(USize n_orbiters, Float central_mass = 1.0,
                                            Float min_radius = 0.5, Float max_radius = 10.0,
                                            Float orbiter_mass = 1e-6, Float g = scale_au::G);

}  // namespace body

namespace generator {

template <FloatT Float>
GenerateDistributionConfig<Float> toy_uniform_box(USize n = 1000);

template <FloatT Float>
GenerateDistributionConfig<Float> toy_uniform_disk(USize n = 1000);

template <FloatT Float>
GenerateDistributionConfig<Float> au_plummer_salpeter(USize n = 1000);

template <FloatT Float>
GenerateDistributionConfig<Float> au_uniform_disk(USize n = 1000);

}  // namespace generator

namespace config {

template <FloatT Float>
typename Direct<Float>::Config direct_toy(std::vector<BodyT<Float>> bodies, bool parallel = false);

template <FloatT Float>
typename Direct<Float>::Config direct_au(std::vector<BodyT<Float>> bodies, bool parallel = false);

template <FloatT Float>
typename Direct<Float>::Config direct_high_accuracy(std::vector<BodyT<Float>> bodies,
                                                    bool                      parallel = false);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_toy(std::vector<BodyT<Float>> bodies,
                                                 bool parallel = false, Float theta = 0.5);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_au(std::vector<BodyT<Float>> bodies,
                                                bool parallel = false, Float theta = 0.5);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_high_accuracy(std::vector<BodyT<Float>> bodies,
                                                           bool  parallel = false,
                                                           Float theta    = 0.3);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_fast(std::vector<BodyT<Float>> bodies,
                                                  bool parallel = false, Float theta = 1.0);

template <FloatT Float, math::MortonCodeT MortonCode = U64>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_toy(
    std::vector<BodyT<Float>> bodies, bool parallel = false, Float theta = 0.5);

template <FloatT Float, math::MortonCodeT MortonCode = U64>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_au(
    std::vector<BodyT<Float>> bodies, bool parallel = false, Float theta = 0.5);

template <FloatT Float, math::MortonCodeT MortonCode = U64>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_high_accuracy(
    std::vector<BodyT<Float>> bodies, bool parallel = false, Float theta = 0.3);

template <FloatT Float, math::MortonCodeT MortonCode = U64>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_fast(
    std::vector<BodyT<Float>> bodies, bool parallel = false, Float theta = 1.0);

}  // namespace config

namespace test {

template <FloatT Float>
std::vector<BodyT<Float>> simple_four_body();

template <FloatT Float>
std::vector<BodyT<Float>> symmetric_square(Float size = 1.0, Float mass = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> two_body_test(Float separation = 1.0, Float mass1 = 1.0,
                                        Float mass2 = 1.0);

template <FloatT Float>
std::vector<BodyT<Float>> close_encounter(Float min_separation = 0.01);

template <FloatT Float>
std::vector<BodyT<Float>> stress_test(USize n = 10000);

}  // namespace test

namespace benchmark {

template <FloatT Float>
std::vector<BodyT<Float>> small(USize n = 100);

template <FloatT Float>
std::vector<BodyT<Float>> medium(USize n = 1000);

template <FloatT Float>
std::vector<BodyT<Float>> large(USize n = 10000);

template <FloatT Float>
std::vector<BodyT<Float>> xlarge(USize n = 100000);

}  // namespace benchmark
}  // namespace nbody::sim::preset
