#include "base/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/direct.hpp"
#include "sim/kepler.hpp"
#include "sim/preset.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace nbody::sim;
using namespace nbody::base::type;

TEST_CASE("Barnes-Hut: two-body matches Kepler analytic solution") {
    using Float = F64;
    using Body  = BodyT<Float>;

    std::vector<Body>        bodies = preset::body::two_body_circular<Float>();
    BarnesHut<Float>::Config bh_cfg = preset::config::barnes_hut_toy<Float>(bodies, false, 0.5);
    BarnesHut<Float>         sim(bh_cfg);

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
        CHECK(sim_bodies[i].pos.is_approx_equal(kep_bodies[i].pos, 1e-2));
        CHECK(sim_bodies[i].vel.is_approx_equal(kep_bodies[i].vel, 1e-2));
    }
}

TEST_CASE("Barnes-Hut: matches direct for small N") {
    using Float                      = F64;
    using Body                       = BodyT<Float>;
    std::vector<Body>        bodies  = preset::test::simple_four_body<Float>();
    BarnesHut<Float>::Config bh_cfg  = preset::config::barnes_hut_toy<Float>(bodies, false, 0.5);
    Direct<Float>::Config    dir_cfg = preset::config::direct_toy<Float>(bodies, false);

    BarnesHut<Float> bh(bh_cfg);
    Direct<Float>    direct(dir_cfg);

    Float dt    = 0.01;
    USize steps = 100;
    for (USize i = 0; i < steps; ++i) {
        bh.step(dt);
        direct.step(dt);
    }

    std::span<const Body, std::dynamic_extent> b_bh  = bh.bodies();
    std::span<const Body, std::dynamic_extent> b_dir = direct.bodies();
    for (USize i = 0; i < b_bh.size(); ++i) {
        CHECK(b_bh[i].pos.is_approx_equal(b_dir[i].pos, 1e-2));
        CHECK(b_bh[i].vel.is_approx_equal(b_dir[i].vel, 1e-2));
    }
}
