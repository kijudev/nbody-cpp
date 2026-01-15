#include <cmath>
#include <numbers>
#include <random>
#include <vector>

#include "base/type.hpp"
#include "math/vec.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/const.hpp"
#include "sim/direct.hpp"
#include "sim/generator.hpp"
#include "sim/integrator.hpp"
#include "sim/preset.hpp"
#include "sim/type.hpp"

namespace nbody::sim::preset {
using namespace nbody::base::type;

namespace body {
template <FloatT Float>
std::vector<BodyT<Float>> two_body_circular(Float central_mass, Float orbiter_mass,
                                            Float orbital_radius, Float g) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(2);

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{0.0, 0.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = central_mass,
    });

    Float orbital_velocity = std::sqrt(g * central_mass / orbital_radius);

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{orbital_radius, 0.0             },
        .vel  = Vec2{0.0,            orbital_velocity},
        .acc  = Vec2{0.0,            0.0             },
        .mass = orbiter_mass,
    });

    return bodies;
}

template std::vector<BodyF32> two_body_circular(F32, F32, F32, F32);
template std::vector<BodyF64> two_body_circular(F64, F64, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> uniform_box(USize n, Float radius, Float min_mass, Float max_mass) {
    GenerateDistributionConfig<Float> config{
        .n           = n,
        .min_mass    = min_mass,
        .max_mass    = max_mass,
        .radius      = radius,
        .position_fn = generate_position_distribution_uniform_box<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };

    return generate_distribution(config);
}

template std::vector<BodyF32> uniform_box(USize, F32, F32, F32);
template std::vector<BodyF64> uniform_box(USize, F64, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> uniform_disk(USize n, Float radius, Float min_mass, Float max_mass) {
    GenerateDistributionConfig<Float> config{
        .n           = n,
        .min_mass    = min_mass,
        .max_mass    = max_mass,
        .radius      = radius,
        .position_fn = generate_position_distribution_uniform_disk<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };

    return generate_distribution(config);
}

template std::vector<BodyF32> uniform_disk(USize, F32, F32, F32);
template std::vector<BodyF64> uniform_disk(USize, F64, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> plummer(USize n, Float radius, Float min_mass, Float max_mass) {
    GenerateDistributionConfig<Float> config{
        .n           = n,
        .min_mass    = min_mass,
        .max_mass    = max_mass,
        .radius      = radius,
        .position_fn = generate_position_distribution_plummer_model<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };

    return generate_distribution(config);
}

template std::vector<BodyF32> plummer(USize, F32, F32, F32);
template std::vector<BodyF64> plummer(USize, F64, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> plummer_salpeter(USize n, Float radius, Float min_mass, Float max_mass) {
    GenerateDistributionConfig<Float> config{
        .n           = n,
        .min_mass    = min_mass,
        .max_mass    = max_mass,
        .radius      = radius,
        .position_fn = generate_position_distribution_plummer_model<Float>,
        .mass_fn     = generate_mass_distribution_salpeter_imf<Float>,
    };

    return generate_distribution(config);
}

template std::vector<BodyF32> plummer_salpeter(USize, F32, F32, F32);
template std::vector<BodyF64> plummer_salpeter(USize, F64, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> grid(USize nx, USize ny, Float spacing, Float mass) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(nx * ny);

    Float offset_x = static_cast<Float>(nx - 1) * spacing / 2.0;
    Float offset_y = static_cast<Float>(ny - 1) * spacing / 2.0;

    for (USize i = 0; i < nx; ++i) {
        for (USize j = 0; j < ny; ++j) {
            Float x = static_cast<Float>(i) * spacing - offset_x;
            Float y = static_cast<Float>(j) * spacing - offset_y;
            bodies.push_back(BodyT<Float>{
                .pos  = Vec2{x,   y  },
                .vel  = Vec2{0.0, 0.0},
                .acc  = Vec2{0.0, 0.0},
                .mass = mass,
            });
        }
    }

    return bodies;
}

template std::vector<BodyF32> grid(USize, USize, F32, F32);
template std::vector<BodyF64> grid(USize, USize, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> ring(USize n, Float radius, Float mass) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(n);

    constexpr Float pi = std::numbers::pi_v<Float>;

    for (USize i = 0; i < n; ++i) {
        Float angle = static_cast<Float>(i) * 2.0 * pi / static_cast<Float>(n);
        Float x     = radius * std::cos(angle);
        Float y     = radius * std::sin(angle);
        bodies.push_back(BodyT<Float>{
            .pos  = Vec2{x,   y  },
            .vel  = Vec2{0.0, 0.0},
            .acc  = Vec2{0.0, 0.0},
            .mass = mass,
        });
    }

    return bodies;
}

template std::vector<BodyF32> ring(USize, F32, F32);
template std::vector<BodyF64> ring(USize, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> solar_system_like(USize n_orbiters, Float central_mass, Float min_radius,
                                            Float max_radius, Float orbiter_mass, Float g) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(n_orbiters + 1);

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{0.0, 0.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = central_mass,
    });

    constexpr Float pi = std::numbers::pi_v<Float>;

    // WHY: THE MEANING OF LIFE, THE UNIVERSE, AND EVERYTHING!!!
    std::default_random_engine            rng(42);
    std::uniform_real_distribution<Float> radius_dist(min_radius, max_radius);
    std::uniform_real_distribution<Float> angle_dist(0.0, 2.0 * pi);

    for (USize i = 0; i < n_orbiters; ++i) {
        Float r     = radius_dist(rng);
        Float angle = angle_dist(rng);

        Float x = r * std::cos(angle);
        Float y = r * std::sin(angle);

        Float v_mag = std::sqrt(g * central_mass / r);
        Float vx    = -v_mag * std::sin(angle);
        Float vy    = v_mag * std::cos(angle);

        bodies.push_back(BodyT<Float>{
            .pos  = Vec2{x,   y  },
            .vel  = Vec2{vx,  vy },
            .acc  = Vec2{0.0, 0.0},
            .mass = orbiter_mass,
        });
    }

    return bodies;
}

template std::vector<BodyF32> solar_system_like(USize, F32, F32, F32, F32, F32);
template std::vector<BodyF64> solar_system_like(USize, F64, F64, F64, F64, F64);

}  // namespace body

namespace generator {

template <FloatT Float>
GenerateDistributionConfig<Float> toy_uniform_box(USize n) {
    return GenerateDistributionConfig<Float>{
        .n           = n,
        .min_mass    = 1.0,
        .max_mass    = 1.0,
        .radius      = 10.0,
        .position_fn = generate_position_distribution_uniform_box<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };
}

template GenerateDistributionConfig<F32> toy_uniform_box(USize);
template GenerateDistributionConfig<F64> toy_uniform_box(USize);

template <FloatT Float>
GenerateDistributionConfig<Float> toy_uniform_disk(USize n) {
    return GenerateDistributionConfig<Float>{
        .n           = n,
        .min_mass    = 1.0,
        .max_mass    = 1.0,
        .radius      = 10.0,
        .position_fn = generate_position_distribution_uniform_disk<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };
}

template GenerateDistributionConfig<F32> toy_uniform_disk(USize);
template GenerateDistributionConfig<F64> toy_uniform_disk(USize);

template <FloatT Float>
GenerateDistributionConfig<Float> au_plummer_salpeter(USize n) {
    return GenerateDistributionConfig<Float>{
        .n           = n,
        .min_mass    = static_cast<Float>(scale_au::MASS_HYGIEA),
        .max_mass    = static_cast<Float>(scale_au::MASS_SOL * 10.0),
        .radius      = static_cast<Float>(scale_au::DISTANCE_AU * 50.0),
        .position_fn = generate_position_distribution_plummer_model<Float>,
        .mass_fn     = generate_mass_distribution_salpeter_imf<Float>,
    };
}

template GenerateDistributionConfig<F32> au_plummer_salpeter(USize);
template GenerateDistributionConfig<F64> au_plummer_salpeter(USize);

template <FloatT Float>
GenerateDistributionConfig<Float> au_uniform_disk(USize n) {
    return GenerateDistributionConfig<Float>{
        .n           = n,
        .min_mass    = static_cast<Float>(scale_au::MASS_EARTH),
        .max_mass    = static_cast<Float>(scale_au::MASS_JUPITER),
        .radius      = static_cast<Float>(scale_au::DISTANCE_AU * 30.0),
        .position_fn = generate_position_distribution_uniform_disk<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };
}

template GenerateDistributionConfig<F32> au_uniform_disk(USize);
template GenerateDistributionConfig<F64> au_uniform_disk(USize);

}  // namespace generator

// ============================================================================
// Simulation Configuration Presets
// ============================================================================

namespace config {

// --- Direct Simulation Configs ---

template <FloatT Float>
typename Direct<Float>::Config direct_toy(std::vector<BodyT<Float>> bodies, bool parallel) {
    return typename Direct<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_toy::G),
        .softening         = static_cast<Float>(scale_toy::SOFTENING),
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template Direct<F32>::Config direct_toy(std::vector<BodyF32>, bool);
template Direct<F64>::Config direct_toy(std::vector<BodyF64>, bool);

template <FloatT Float>
typename Direct<Float>::Config direct_au(std::vector<BodyT<Float>> bodies, bool parallel) {
    return typename Direct<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING),
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template Direct<F32>::Config direct_au(std::vector<BodyF32>, bool);
template Direct<F64>::Config direct_au(std::vector<BodyF64>, bool);

template <FloatT Float>
typename Direct<Float>::Config direct_high_accuracy(std::vector<BodyT<Float>> bodies,
                                                    bool                      parallel) {
    return typename Direct<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING * 0.1),
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template Direct<F32>::Config direct_high_accuracy(std::vector<BodyF32>, bool);
template Direct<F64>::Config direct_high_accuracy(std::vector<BodyF64>, bool);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_toy(std::vector<BodyT<Float>> bodies, bool parallel,
                                                 Float theta) {
    return typename BarnesHut<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_toy::G),
        .softening         = static_cast<Float>(scale_toy::SOFTENING),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHut<F32>::Config barnes_hut_toy(std::vector<BodyF32>, bool, F32);
template BarnesHut<F64>::Config barnes_hut_toy(std::vector<BodyF64>, bool, F64);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_au(std::vector<BodyT<Float>> bodies, bool parallel,
                                                Float theta) {
    return typename BarnesHut<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHut<F32>::Config barnes_hut_au(std::vector<BodyF32>, bool, F32);
template BarnesHut<F64>::Config barnes_hut_au(std::vector<BodyF64>, bool, F64);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_high_accuracy(std::vector<BodyT<Float>> bodies,
                                                           bool parallel, Float theta) {
    return typename BarnesHut<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING * 0.1),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHut<F32>::Config barnes_hut_high_accuracy(std::vector<BodyF32>, bool, F32);
template BarnesHut<F64>::Config barnes_hut_high_accuracy(std::vector<BodyF64>, bool, F64);

template <FloatT Float>
typename BarnesHut<Float>::Config barnes_hut_fast(std::vector<BodyT<Float>> bodies, bool parallel,
                                                  Float theta) {
    return typename BarnesHut<Float>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHut<F32>::Config barnes_hut_fast(std::vector<BodyF32>, bool, F32);
template BarnesHut<F64>::Config barnes_hut_fast(std::vector<BodyF64>, bool, F64);

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_toy(
    std::vector<BodyT<Float>> bodies, bool parallel, Float theta) {
    return typename BarnesHutMorton<Float, MortonCode>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_toy::G),
        .softening         = static_cast<Float>(scale_toy::SOFTENING),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHutMorton<F32, U32>::Config barnes_hut_morton_toy<F32, U32>(std::vector<BodyF32>,
                                                                           bool, F32);
template BarnesHutMorton<F32, U64>::Config barnes_hut_morton_toy<F32, U64>(std::vector<BodyF32>,
                                                                           bool, F32);
template BarnesHutMorton<F64, U32>::Config barnes_hut_morton_toy<F64, U32>(std::vector<BodyF64>,
                                                                           bool, F64);
template BarnesHutMorton<F64, U64>::Config barnes_hut_morton_toy<F64, U64>(std::vector<BodyF64>,
                                                                           bool, F64);

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_au(
    std::vector<BodyT<Float>> bodies, bool parallel, Float theta) {
    return typename BarnesHutMorton<Float, MortonCode>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHutMorton<F32, U32>::Config barnes_hut_morton_au<F32, U32>(std::vector<BodyF32>,
                                                                          bool, F32);
template BarnesHutMorton<F32, U64>::Config barnes_hut_morton_au<F32, U64>(std::vector<BodyF32>,
                                                                          bool, F32);
template BarnesHutMorton<F64, U32>::Config barnes_hut_morton_au<F64, U32>(std::vector<BodyF64>,
                                                                          bool, F64);
template BarnesHutMorton<F64, U64>::Config barnes_hut_morton_au<F64, U64>(std::vector<BodyF64>,
                                                                          bool, F64);

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_high_accuracy(
    std::vector<BodyT<Float>> bodies, bool parallel, Float theta) {
    return typename BarnesHutMorton<Float, MortonCode>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING * 0.1),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHutMorton<F32, U32>::Config barnes_hut_morton_high_accuracy<F32, U32>(
    std::vector<BodyF32>, bool, F32);
template BarnesHutMorton<F32, U64>::Config barnes_hut_morton_high_accuracy<F32, U64>(
    std::vector<BodyF32>, bool, F32);
template BarnesHutMorton<F64, U32>::Config barnes_hut_morton_high_accuracy<F64, U32>(
    std::vector<BodyF64>, bool, F64);
template BarnesHutMorton<F64, U64>::Config barnes_hut_morton_high_accuracy<F64, U64>(
    std::vector<BodyF64>, bool, F64);

template <FloatT Float, math::MortonCodeT MortonCode>
typename BarnesHutMorton<Float, MortonCode>::Config barnes_hut_morton_fast(
    std::vector<BodyT<Float>> bodies, bool parallel, Float theta) {
    return typename BarnesHutMorton<Float, MortonCode>::Config{
        .bodies            = std::move(bodies),
        .g                 = static_cast<Float>(scale_au::G),
        .softening         = static_cast<Float>(scale_au::SOFTENING),
        .theta             = theta,
        .parallel          = parallel,
        .use_proper_verlet = true,
        .integrate_fn      = integrate_body_verlet<Float>,
    };
}

template BarnesHutMorton<F32, U32>::Config barnes_hut_morton_fast<F32, U32>(std::vector<BodyF32>,
                                                                            bool, F32);
template BarnesHutMorton<F32, U64>::Config barnes_hut_morton_fast<F32, U64>(std::vector<BodyF32>,
                                                                            bool, F32);
template BarnesHutMorton<F64, U32>::Config barnes_hut_morton_fast<F64, U32>(std::vector<BodyF64>,
                                                                            bool, F64);
template BarnesHutMorton<F64, U64>::Config barnes_hut_morton_fast<F64, U64>(std::vector<BodyF64>,
                                                                            bool, F64);

}  // namespace config

namespace test {

template <FloatT Float>
std::vector<BodyT<Float>> simple_four_body() {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(4);

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{0.0, 0.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = static_cast<Float>(1.0),
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{1.0, 0.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = static_cast<Float>(1.0),
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{0.0, 1.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = static_cast<Float>(1.0),
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{1.0, 1.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = static_cast<Float>(1.0),
    });

    return bodies;
}

template std::vector<BodyF32> simple_four_body();
template std::vector<BodyF64> simple_four_body();

template <FloatT Float>
std::vector<BodyT<Float>> symmetric_square(Float size, Float mass) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(4);

    Float half = size / 2.0;

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{-half, -half},
        .vel  = Vec2{0.0,   0.0  },
        .acc  = Vec2{0.0,   0.0  },
        .mass = mass,
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{half, -half},
        .vel  = Vec2{0.0,  0.0  },
        .acc  = Vec2{0.0,  0.0  },
        .mass = mass,
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{-half, half},
        .vel  = Vec2{0.0,   0.0 },
        .acc  = Vec2{0.0,   0.0 },
        .mass = mass,
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{half, half},
        .vel  = Vec2{0.0,  0.0 },
        .acc  = Vec2{0.0,  0.0 },
        .mass = mass,
    });

    return bodies;
}

template std::vector<BodyF32> symmetric_square(F32, F32);
template std::vector<BodyF64> symmetric_square(F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> two_body_test(Float separation, Float mass1, Float mass2) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(2);

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{0.0, 0.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = mass1,
    });
    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{separation, 0.0},
        .vel  = Vec2{0.0,        0.0},
        .acc  = Vec2{0.0,        0.0},
        .mass = mass2,
    });

    return bodies;
}

template std::vector<BodyF32> two_body_test(F32, F32, F32);
template std::vector<BodyF64> two_body_test(F64, F64, F64);

template <FloatT Float>
std::vector<BodyT<Float>> close_encounter(Float min_separation) {
    using Vec2 = math::Vec2T<Float>;

    std::vector<BodyT<Float>> bodies;
    bodies.reserve(3);

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{0.0, 0.0},
        .vel  = Vec2{0.0, 0.0},
        .acc  = Vec2{0.0, 0.0},
        .mass = static_cast<Float>(1.0),
    });

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{min_separation, 0.0},
        .vel  = Vec2{0.0,            0.0},
        .acc  = Vec2{0.0,            0.0},
        .mass = static_cast<Float>(1.0),
    });

    bodies.push_back(BodyT<Float>{
        .pos  = Vec2{10.0, 10.0},
        .vel  = Vec2{0.0,  0.0 },
        .acc  = Vec2{0.0,  0.0 },
        .mass = static_cast<Float>(1.0),
    });

    return bodies;
}

template std::vector<BodyF32> close_encounter(F32);
template std::vector<BodyF64> close_encounter(F64);

template <FloatT Float>
std::vector<BodyT<Float>> stress_test(USize n) {
    return body::uniform_box<Float>(n, static_cast<Float>(100.0), static_cast<Float>(0.1),
                                    static_cast<Float>(10.0));
}

template std::vector<BodyF32> stress_test(USize);
template std::vector<BodyF64> stress_test(USize);

}  // namespace test

namespace benchmark {

template <FloatT Float>
std::vector<BodyT<Float>> small(USize n) {
    return body::plummer<Float>(n, static_cast<Float>(10.0), static_cast<Float>(1.0),
                                static_cast<Float>(1.0));
}

template std::vector<BodyF32> small(USize);
template std::vector<BodyF64> small(USize);

template <FloatT Float>
std::vector<BodyT<Float>> medium(USize n) {
    return body::plummer<Float>(n, static_cast<Float>(50.0), static_cast<Float>(0.5),
                                static_cast<Float>(5.0));
}

template std::vector<BodyF32> medium(USize);
template std::vector<BodyF64> medium(USize);

template <FloatT Float>
std::vector<BodyT<Float>> large(USize n) {
    return body::plummer<Float>(n, static_cast<Float>(100.0), static_cast<Float>(0.1),
                                static_cast<Float>(10.0));
}

template std::vector<BodyF32> large(USize);
template std::vector<BodyF64> large(USize);

template <FloatT Float>
std::vector<BodyT<Float>> xlarge(USize n) {
    return body::plummer<Float>(n, static_cast<Float>(200.0), static_cast<Float>(0.01),
                                static_cast<Float>(100.0));
}

template std::vector<BodyF32> xlarge(USize);
template std::vector<BodyF64> xlarge(USize);

}  // namespace benchmark
}  // namespace nbody::sim::preset
