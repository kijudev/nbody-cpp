#include "base/type.hpp"
#include "sim/const.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <vector>

#include "math/vec.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/barnes_hut_linear.hpp"
#include "sim/direct.hpp"
#include "sim/generator.hpp"
#include "sim/type.hpp"

using namespace nbody::base::type;
using namespace nbody::sim;

TEST_CASE("barnes_hut vs direct (uniform box) - F64") {
    using Float = F64;

    static constexpr Float G         = scale_toy::G;
    static constexpr Float SOFTENING = scale_toy::SOFTENING;

    GenerateDistributionConfig<Float> gen_config{
        .n           = 64,
        .min_mass    = 0.1,
        .max_mass    = 10.0,
        .radius      = static_cast<Float>(10.0),
        .position_fn = generate_position_distribution_uniform_box<Float>,
        .mass_fn     = generate_mass_distribution_uniform<Float>,
    };

    std::vector<BodyT<Float>> bodies = generate_distribution(gen_config);

    Direct<Float>::Config direct_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
    };

    BarnesHut<Float>::Config barnes_hut_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = static_cast<Float>(G),
        .softening    = static_cast<Float>(SOFTENING),
        .theta        = static_cast<Float>(1e-6),  // NOTE: Tighter theta for more precision.
        .depth        = 64,
    };

    BarnesHutLinear<Float>::Config barnes_hut_linear_config{
        .bodies        = bodies,
        .integrate_fn  = integrate_body_euler<Float>,
        .g             = static_cast<Float>(G),
        .softening     = static_cast<Float>(SOFTENING),
        .theta         = static_cast<Float>(1e-6),  // NOTE: Tighter theta for more precision.
        .depth         = 64,
        .reserve_nodes = 1024,
    };

    Direct<Float>          direct(direct_config);
    BarnesHut<Float>       barnes_hut(barnes_hut_config);
    BarnesHutLinear<Float> barnes_hut_linear(barnes_hut_linear_config);

    direct.step(0.0);
    barnes_hut.step(0.0);
    barnes_hut_linear.step(0.0);

    auto direct_bodies = direct.bodies();
    auto bh_bodies     = barnes_hut.bodies();
    auto bhl_bodies    = barnes_hut_linear.bodies();

    REQUIRE(direct_bodies.size() == bh_bodies.size());
    REQUIRE(direct_bodies.size() == bhl_bodies.size());

    // NOTE: Compute maximum absolute acceleration difference between direct and each Barnes-Hut
    // implementation.
    Float max_abs_err_bh  = 0.0;
    Float max_abs_err_bhl = 0.0;

    for (USize i = 0; i < direct_bodies.size(); ++i) {
        const auto& a = direct_bodies[i].acc;
        const auto& b = bh_bodies[i].acc;
        const auto& c = bhl_bodies[i].acc;

        max_abs_err_bh  = std::max(max_abs_err_bh, a.distance(b));
        max_abs_err_bhl = std::max(max_abs_err_bhl, a.distance(c));
    }

    // NOTE: Allow somewhat loose tolerance to account for differences in traversal order/rounding.
    const Float tolerance = static_cast<Float>(1e-2);

    CHECK(max_abs_err_bh < tolerance);
    CHECK(max_abs_err_bhl < tolerance);

    // NOTE: Ensure the two Barnes-Hut implementations agree closely with each other.
    Float max_abs_err_bh_pair = 0.0;
    for (std::size_t i = 0; i < direct_bodies.size(); ++i) {
        max_abs_err_bh_pair =
            std::max(max_abs_err_bh_pair, bh_bodies[i].acc.distance(bhl_bodies[i].acc));
    }

    // NOTE: Barnes-Hut pointer and linear should be almost identical, allow tighter tolerance.
    const Float tolarance_pair = static_cast<Float>(1e-8);
    CHECK(max_abs_err_bh_pair < tolarance_pair);
}

TEST_CASE("barnes_hut vs direct (plummer) - F64") {
    using Float = F64;

    static constexpr Float G         = scale_toy::G;
    static constexpr Float SOFTENING = scale_toy::SOFTENING;

    // NOTE: Use Plummer distribution which is more clustered.
    GenerateDistributionConfig<Float> generate_distribution_config{
        .n           = 48,
        .min_mass    = 0.5,
        .max_mass    = 5.0,
        .radius      = static_cast<Float>(5.0),
        .position_fn = generate_position_distribution_plummer_model<Float>,
        .mass_fn     = generate_mass_distribution_salpeter_imf<Float>,
    };

    std::vector<BodyT<Float>> bodies = generate_distribution(generate_distribution_config);

    Direct<Float>::Config direct_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = G,
        .softening    = SOFTENING,
    };

    BarnesHut<Float>::Config barnes_hut_config{
        .bodies       = bodies,
        .integrate_fn = integrate_body_euler<Float>,
        .g            = G,
        .softening    = SOFTENING,
        .theta        = 0.15,  // NOTE: Tighter theta for clustered data.
        .depth        = 64,
    };

    BarnesHutLinear<Float>::Config barnes_hut_linear_config{
        .bodies        = bodies,
        .integrate_fn  = integrate_body_euler<Float>,
        .g             = G,
        .softening     = SOFTENING,
        .theta         = 0.15,  // NOTE: Tighter theta for clustered data.
        .depth         = 64,
        .reserve_nodes = 1024,
    };

    Direct<Float>          direct(direct_config);
    BarnesHut<Float>       barnes_hut(barnes_hut_config);
    BarnesHutLinear<Float> barnes_hut_linear(barnes_hut_linear_config);

    direct.step(0.0);
    barnes_hut.step(0.0);
    barnes_hut_linear.step(0.0);

    auto direct_bodies = direct.bodies();
    auto bh_bodies     = barnes_hut.bodies();
    auto bhl_bodies    = barnes_hut_linear.bodies();

    REQUIRE(direct_bodies.size() == bh_bodies.size());
    REQUIRE(direct_bodies.size() == bhl_bodies.size());

    // NOTE: Compute maximum absolute acceleration difference between direct and each Barnes-Hut
    // implementation.
    Float max_abs_err_bh  = 0.0;
    Float max_abs_err_bhl = 0.0;

    for (USize i = 0; i < direct_bodies.size(); ++i) {
        const auto& a = direct_bodies[i].acc;
        const auto& b = bh_bodies[i].acc;
        const auto& c = bhl_bodies[i].acc;

        max_abs_err_bh  = std::max(max_abs_err_bh, a.distance(b));
        max_abs_err_bhl = std::max(max_abs_err_bhl, a.distance(c));
    }

    // NOTE: Looser tolerance for clustered plummer model but still should be reasonably close.
    const Float tolerance = 5e-4;

    CHECK(max_abs_err_bh < tolerance);
    CHECK(max_abs_err_bhl < tolerance);

    // NOTE: Barnes-Hut pointer and linear consistency.
    Float max_abs_err_bh_pair = 0.0;
    for (USize i = 0; i < direct_bodies.size(); ++i) {
        max_abs_err_bh_pair =
            std::max(max_abs_err_bh_pair, bh_bodies[i].acc.distance(bhl_bodies[i].acc));
    }
    const Float tol_pair = 1e-8;
    CHECK(max_abs_err_bh_pair < tol_pair);
}
