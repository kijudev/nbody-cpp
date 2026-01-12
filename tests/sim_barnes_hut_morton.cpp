#include "base/type.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/direct.hpp"
#include "sim/kepler.hpp"
#include "sim/preset.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace nbody::sim;
using namespace nbody::base::type;

TEST_CASE("Barnes-Hut-Morton: two-body matches Kepler analytic solution") {
    using Float = F64;
    using Body  = BodyT<Float>;

    std::vector<Body>              bodies = preset::body::two_body_circular<Float>();
    BarnesHutMorton<Float>::Config bhm_cfg =
        preset::config::barnes_hut_morton_toy<Float>(bodies, false, 0.5);
    BarnesHutMorton<Float> sim(bhm_cfg);

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

TEST_CASE("Barnes-Hut-Morton: matches direct for small N") {
    using Float = F64;
    using Body  = BodyT<Float>;

    std::vector<Body>              bodies = preset::test::simple_four_body<Float>();
    BarnesHutMorton<Float>::Config bhm_cfg =
        preset::config::barnes_hut_morton_toy<Float>(bodies, false, 0.5);
    Direct<Float>::Config dir_cfg = preset::config::direct_toy<Float>(bodies, false);

    BarnesHutMorton<Float> bhm(bhm_cfg);
    Direct<Float>          direct(dir_cfg);

    Float dt    = 0.01;
    USize steps = 100;
    for (USize i = 0; i < steps; ++i) {
        bhm.step(dt);
        direct.step(dt);
    }

    std::span<const Body, std::dynamic_extent> b_bhm = bhm.bodies();
    std::span<const Body, std::dynamic_extent> b_dir = direct.bodies();

    CHECK(b_bhm.size() == b_dir.size());

    for (USize i = 0; i < b_bhm.size(); ++i) {
        CHECK(b_bhm[i].pos.is_approx_equal(b_dir[i].pos, 1e-2));
        CHECK(b_bhm[i].vel.is_approx_equal(b_dir[i].vel, 1e-2));
    }
}
