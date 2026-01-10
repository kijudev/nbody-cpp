#include "base/type.hpp"
#include "sim/const.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <vector>

#include "math/vec.hpp"
#include "sim/barnes_hut_linear.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/direct.hpp"
#include "sim/generator.hpp"
#include "sim/type.hpp"

using namespace nbody::base::type;
using namespace nbody::sim;

TEST_CASE("BarnesHutMorton - Direct as baseline - Uniform Box Distribution - F64") {
    using Float                       = F64;
    static constexpr USize BODY_COUNT = 256;
    static constexpr Float G          = scale_toy::G;
    static constexpr Float SOFTENING  = scale_toy::SOFTENING;

    std::vector<BodyT<Float>> bodies = generate_distribution(GenerateDistributionConfig<Float>{
        .n           = BODY_COUNT,
        .min_mass    = 0.1,
        .max_mass    = 10.0,
        .radius      = static_cast<Float>(10.0),
        .position_fn = generate_position_distribution_uniform_box<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    });

    Direct<Float>::Config direct_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
    };

    BarnesHutMorton<Float, U64>::Config bhm_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
        .theta        = static_cast<Float>(1e-6),  // NOTE: Tighter theta for more precision.
    };

    Direct<Float>               direct(direct_config);
    BarnesHutMorton<Float, U64> bhm(bhm_config);

    direct.step(0.0);
    bhm.step(0.0);

    auto direct_bodies = direct.bodies();
    auto bhm_bodies    = bhm.bodies();

    REQUIRE(direct_bodies.size() == bhm_bodies.size());

    // NOTE: Compute maximum absolute acceleration difference between direct and Barnes-Hut Morton.
    Float max_abs_err_bhm = 0.0;

    for (USize i = 0; i < direct_bodies.size(); ++i) {
        const auto& a = direct_bodies[i].acc;
        const auto& b = bhm_bodies[i].acc;

        max_abs_err_bhm = std::max(max_abs_err_bhm, a.distance(b));
    }

    // NOTE: Allow somewhat loose tolerance to account for differences in traversal order/rounding.
    const Float tolerance = 1e-2;

    CHECK(max_abs_err_bhm < tolerance);
}

TEST_CASE("BarnesHutMorton vs BarnesHutLinear - Uniform Disk Distribution - F64") {
    using Float                       = F64;
    static constexpr USize BODY_COUNT = 128;
    static constexpr Float G          = scale_toy::G;
    static constexpr Float SOFTENING  = scale_toy::SOFTENING;

    std::vector<BodyT<Float>> bodies = generate_distribution(GenerateDistributionConfig<Float>{
        .n           = BODY_COUNT,
        .min_mass    = 0.5,
        .max_mass    = 5.0,
        .radius      = static_cast<Float>(15.0),
        .position_fn = generate_position_distribution_uniform_disk<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    });

    BarnesHutLinear<Float>::Config bhl_config{
        .bodies        = bodies,
        .integrate_fn  = integrate_body_euler<Float>,
        .g             = static_cast<Float>(G),
        .softening     = static_cast<Float>(SOFTENING),
        .theta         = static_cast<Float>(0.5),
        .depth         = 64,
        .reserve_nodes = 1024,
    };

    BarnesHutMorton<Float, U64>::Config bhm_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
        .theta        = static_cast<Float>(0.5),
    };

    BarnesHutLinear<Float>      bhl(bhl_config);
    BarnesHutMorton<Float, U64> bhm(bhm_config);

    bhl.step(0.0);
    bhm.step(0.0);

    auto bhl_bodies = bhl.bodies();
    auto bhm_bodies = bhm.bodies();

    REQUIRE(bhl_bodies.size() == bhm_bodies.size());

    // NOTE: Compare accelerations between the two implementations.
    Float max_abs_err = 0.0;

    for (USize i = 0; i < bhl_bodies.size(); ++i) {
        const auto& a = bhl_bodies[i].acc;
        const auto& b = bhm_bodies[i].acc;

        max_abs_err = std::max(max_abs_err, a.distance(b));
    }

    // NOTE: Allow reasonable tolerance as tree structures may differ slightly.
    const Float tolerance = 1e-3;

    CHECK(max_abs_err < tolerance);
}

TEST_CASE("BarnesHutMorton - Small system sanity check - F64") {
    using Float = F64;

    std::vector<BodyT<Float>> bodies = {
        BodyT<Float>{.pm = {.pos = {0.0, 0.0}, .mass = 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}},
        BodyT<Float>{.pm = {.pos = {1.0, 0.0}, .mass = 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}},
        BodyT<Float>{.pm = {.pos = {0.0, 1.0}, .mass = 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}},
    };

    BarnesHutMorton<Float, U32>::Config config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = 1.0,
        .softening    = 0.01,
        .theta        = 0.5,
    };

    BarnesHutMorton<Float, U32> sim(config);

    sim.step(0.0);

    auto result_bodies = sim.bodies();

    REQUIRE(result_bodies.size() == 3);

    // NOTE: Check that accelerations are non-zero (bodies should attract each other).
    for (const auto& body : result_bodies) {
        Float acc_magnitude = body.acc.length();
        CHECK(acc_magnitude > 0.0);
    }
}

TEST_CASE("BarnesHutMorton - Empty bodies list - F64") {
    using Float = F64;

    std::vector<BodyT<Float>> bodies = {};

    BarnesHutMorton<Float, U64>::Config config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = scale_toy::G,
        .softening    = scale_toy::SOFTENING,
        .theta        = 0.5,
    };

    BarnesHutMorton<Float, U64> sim(config);

    // NOTE: Should handle empty bodies gracefully.
    REQUIRE_NOTHROW(sim.step(0.01));

    auto result_bodies = sim.bodies();
    CHECK(result_bodies.empty());
}

TEST_CASE("BarnesHutMorton - Single body - F64") {
    using Float = F64;

    std::vector<BodyT<Float>> bodies = {
        BodyT<Float>{.pm = {.pos = {0.0, 0.0}, .mass = 1.0}, .vel = {1.0, 1.0}, .acc = {0.0, 0.0}},
    };

    BarnesHutMorton<Float, U32>::Config config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = 1.0,
        .softening    = 0.01,
        .theta        = 0.5,
    };

    BarnesHutMorton<Float, U32> sim(config);

    sim.step(0.1);

    auto result_bodies = sim.bodies();

    REQUIRE(result_bodies.size() == 1);

    // NOTE: Single body should have zero acceleration and should move with constant velocity.
    CHECK(result_bodies[0].acc.length() < 1e-10);
    CHECK(result_bodies[0].pm.pos.x > 0.0);  // Should have moved
    CHECK(result_bodies[0].pm.pos.y > 0.0);
}

TEST_CASE("BarnesHutMorton - Large system stress test - F64") {
    using Float                       = F64;
    static constexpr USize BODY_COUNT = 1024;
    static constexpr Float G          = scale_toy::G;
    static constexpr Float SOFTENING  = scale_toy::SOFTENING;

    std::vector<BodyT<Float>> bodies = generate_distribution(GenerateDistributionConfig<Float>{
        .n           = BODY_COUNT,
        .min_mass    = 0.1,
        .max_mass    = 10.0,
        .radius      = static_cast<Float>(20.0),
        .position_fn = generate_position_distribution_plummer_model<Float>,
        .mass_fn     = generate_mass_distribution_salpeter_imf<Float>,
    });

    BarnesHutMorton<Float, U64>::Config bhm_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
        .theta        = static_cast<Float>(0.5),
    };

    BarnesHutMorton<Float, U64> bhm(bhm_config);

    // NOTE: Run multiple steps to ensure stability.
    REQUIRE_NOTHROW(bhm.step(0.01));
    REQUIRE_NOTHROW(bhm.step(0.01));
    REQUIRE_NOTHROW(bhm.step(0.01));

    auto result_bodies = bhm.bodies();
    CHECK(result_bodies.size() == BODY_COUNT);

    // NOTE: Verify no NaN or infinite values.
    for (const auto& body : result_bodies) {
        CHECK(std::isfinite(body.pm.pos.x));
        CHECK(std::isfinite(body.pm.pos.y));
        CHECK(std::isfinite(body.vel.x));
        CHECK(std::isfinite(body.vel.y));
        CHECK(std::isfinite(body.acc.x));
        CHECK(std::isfinite(body.acc.y));
    }
}

TEST_CASE("BarnesHutMorton - Bodies at same location - F64") {
    using Float = F64;

    std::vector<BodyT<Float>> bodies = {
        BodyT<Float>{.pm = {.pos = {0.0, 0.0}, .mass = 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}},
        BodyT<Float>{.pm = {.pos = {0.0, 0.0}, .mass = 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}},
        BodyT<Float>{.pm = {.pos = {1.0, 1.0}, .mass = 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}},
    };

    BarnesHutMorton<Float, U32>::Config config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = 1.0,
        .softening    = 0.01,
        .theta        = 0.5,
    };

    BarnesHutMorton<Float, U32> sim(config);

    // NOTE: Should handle overlapping bodies gracefully.
    REQUIRE_NOTHROW(sim.step(0.0));

    auto result_bodies = sim.bodies();
    REQUIRE(result_bodies.size() == 3);

    // NOTE: Check for finite values.
    for (const auto& body : result_bodies) {
        CHECK(std::isfinite(body.acc.x));
        CHECK(std::isfinite(body.acc.y));
    }
}

TEST_CASE("BarnesHutMorton U32 vs U64 - consistency check - F64") {
    using Float                       = F64;
    static constexpr USize BODY_COUNT = 64;
    static constexpr Float G          = scale_toy::G;
    static constexpr Float SOFTENING  = scale_toy::SOFTENING;

    std::vector<BodyT<Float>> bodies = generate_distribution(GenerateDistributionConfig<Float>{
        .n           = BODY_COUNT,
        .min_mass    = 0.5,
        .max_mass    = 5.0,
        .radius      = static_cast<Float>(10.0),
        .position_fn = generate_position_distribution_uniform_disk<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    });

    BarnesHutMorton<Float, U32>::Config config_u32{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
        .theta        = static_cast<Float>(0.5),
    };

    BarnesHutMorton<Float, U64>::Config config_u64{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
        .theta        = static_cast<Float>(0.5),
    };

    BarnesHutMorton<Float, U32> sim_u32(config_u32);
    BarnesHutMorton<Float, U64> sim_u64(config_u64);

    sim_u32.step(0.0);
    sim_u64.step(0.0);

    auto bodies_u32 = sim_u32.bodies();
    auto bodies_u64 = sim_u64.bodies();

    REQUIRE(bodies_u32.size() == bodies_u64.size());

    // NOTE: Both Morton code types should produce similar results.
    Float max_abs_err = 0.0;

    for (USize i = 0; i < bodies_u32.size(); ++i) {
        const auto& a = bodies_u32[i].acc;
        const auto& b = bodies_u64[i].acc;

        max_abs_err = std::max(max_abs_err, a.distance(b));
    }

    // NOTE: U64 has better precision, but results should be close.
    const Float tolerance = 0.1;

    CHECK(max_abs_err < tolerance);
}
