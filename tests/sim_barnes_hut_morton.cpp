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

    // NOTE: Relaxed tolerance for Barnes-Hut approximation over many steps
    for (USize i = 0; i < 2; ++i) {
        CHECK(sim_bodies[i].pos.is_approx_equal(kep_bodies[i].pos, 5e-2));
        CHECK(sim_bodies[i].vel.is_approx_equal(kep_bodies[i].vel, 5e-2));
    }
}

TEST_CASE("Barnes-Hut-Morton: two-body short term matches Kepler precisely") {
    using Float = F64;
    using Body  = BodyT<Float>;

    std::vector<Body>              bodies = preset::body::two_body_circular<Float>();
    BarnesHutMorton<Float>::Config bhm_cfg =
        preset::config::barnes_hut_morton_toy<Float>(bodies, false, 0.5);
    BarnesHutMorton<Float> sim(bhm_cfg);

    Kepler<Float> kepler({.body1 = bodies[0], .body2 = bodies[1]});

    Float dt    = 0.01;
    USize steps = 100;  // Shorter integration
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

TEST_CASE("Barnes-Hut-Morton: tree construction sanity check") {
    using Float = F64;
    using Body  = BodyT<Float>;

    // Create 4 bodies in different quadrants
    std::vector<Body> bodies = {
        Body{.pos = {1.0, 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}, .mass = 1.0},
        Body{.pos = {-1.0, 1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}, .mass = 1.0},
        Body{.pos = {-1.0, -1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}, .mass = 1.0},
        Body{.pos = {1.0, -1.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}, .mass = 1.0},
    };

    BarnesHutMorton<Float>::Config cfg{
        .bodies    = bodies,
        .g         = scale_toy::G,
        .softening = scale_toy::SOFTENING,
        .theta     = 0.5,
        .parallel  = false,
    };
    BarnesHutMorton<Float> sim(cfg);

    // Do one step to build tree
    sim.step(0.01);

    const auto& nodes = sim.nodes();
    CHECK(nodes.size() >= 4);  // At least 4 leaf nodes

    // Check total mass is correct
    Float total_mass = 0.0;
    for (const auto& node : nodes) {
        if (node.is_leaf) {
            total_mass += node.mass;
        }
    }
    CHECK(total_mass == doctest::Approx(4.0).epsilon(1e-10));
}

TEST_CASE("Barnes-Hut-Morton: acceleration computation") {
    using Float = F64;
    using Body  = BodyT<Float>;

    // Two body problem - check acceleration is computed
    std::vector<Body> bodies = {
        Body{.pos = {0.0, 0.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}, .mass = 1.0},
        Body{.pos = {1.0, 0.0}, .vel = {0.0, 0.0}, .acc = {0.0, 0.0}, .mass = 1.0},
    };

    BarnesHutMorton<Float>::Config cfg{
        .bodies    = bodies,
        .g         = 1.0,  // Simple G for testing
        .softening = 0.0,
        .theta     = 0.5,
        .parallel  = false,
    };
    BarnesHutMorton<Float> sim(cfg);

    sim.step(0.0);  // dt=0 to just compute acceleration without moving

    auto sim_bodies = sim.bodies();

    // Body 0 should be accelerated towards body 1 (positive x)
    CHECK(sim_bodies[0].acc.x > 0.0);
    CHECK(std::abs(sim_bodies[0].acc.y) < 1e-10);

    // Body 1 should be accelerated towards body 0 (negative x)
    CHECK(sim_bodies[1].acc.x < 0.0);
    CHECK(std::abs(sim_bodies[1].acc.y) < 1e-10);

    // Accelerations should be equal and opposite (by Newton's 3rd law, per unit mass)
    // a = G * M / r^2, with r=1, G=1, M=1: a = 1
    CHECK(sim_bodies[0].acc.x == doctest::Approx(1.0).epsilon(1e-6));
    CHECK(sim_bodies[1].acc.x == doctest::Approx(-1.0).epsilon(1e-6));
}