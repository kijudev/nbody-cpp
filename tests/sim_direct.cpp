#include "base/type.hpp"
#include "sim/direct.hpp"
#include "sim/kepler.hpp"
#include "sim/preset.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace nbody::sim;
using namespace nbody::base::type;

TEST_CASE("Direct: two-body matches Kepler analytic solution") {
    using Float = F64;
    using Body  = BodyT<Float>;

    std::vector<Body>     bodies     = preset::body::two_body_circular<Float>();
    Direct<Float>::Config direct_cfg = preset::config::direct_toy<Float>(bodies, false);
    Direct<Float>         sim(direct_cfg);

    Kepler<Float> kepler({.body1 = bodies[0], .body2 = bodies[1]});

    Float dt    = 0.01;
    USize steps = 1000;
    for (USize i = 0; i < steps; ++i) {
        sim.step(dt);
        kepler.step(dt);
    }

    std::span<const Body, std::dynamic_extent> sim_bodies = sim.bodies();
    std::span<const Body, std::dynamic_extent> kep_bodies = kepler.bodies();

    CHECK(sim_bodies.size() == 2);
    CHECK(kep_bodies.size() == 2);

    for (USize i = 0; i < 2; ++i) {
        CHECK(sim_bodies[i].pos.is_approx_equal(kep_bodies[i].pos, 1e-3));
        CHECK(sim_bodies[i].vel.is_approx_equal(kep_bodies[i].vel, 1e-3));
    }
}

TEST_CASE("Direct: matches itself for small N") {
    using Float = F64;
    using Body  = BodyT<Float>;

    std::vector<Body>     bodies = preset::test::simple_four_body<Float>();
    Direct<Float>::Config cfg    = preset::config::direct_toy<Float>(bodies, false);
    Direct<Float>         sim1(cfg), sim2(cfg);

    Float dt    = 0.01;
    USize steps = 100;
    for (USize i = 0; i < steps; ++i) {
        sim1.step(dt);
        sim2.step(dt);
    }

    std::span<const Body, std::dynamic_extent> b1 = sim1.bodies();
    std::span<const Body, std::dynamic_extent> b2 = sim2.bodies();

    for (USize i = 0; i < b1.size(); ++i) {
        CHECK(b1[i].pos.is_approx_equal(b2[i].pos, 1e-10));
        CHECK(b1[i].vel.is_approx_equal(b2[i].vel, 1e-10));
    }
}
