#include <nanobench.h>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "base/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/barnes_hut_linear.hpp"
#include "sim/direct.hpp"
#include "sim/generator.hpp"
#include "sim/type.hpp"

int main() {
    using namespace nbody::base::type;
    using namespace nbody::sim;
    using Float = F64;

    ankerl::nanobench::Bench bench;
    bench.title("N-Body: Direct vs Barnes-Hut (pointer) vs Barnes-Hut (linear)");
    bench.minEpochTime(std::chrono::milliseconds(50));
    bench.warmup(3);

    std::vector<USize> sizes = {128, 512, 2048, 4096, 8192, 16384};

    for (USize n : sizes) {
        GenerateDistributionConfig<Float> gen_cfg{};
        gen_cfg.n           = n;
        gen_cfg.radius      = static_cast<Float>(10.0);
        gen_cfg.min_mass    = static_cast<Float>(1.0);
        gen_cfg.max_mass    = static_cast<Float>(2.0);
        gen_cfg.position_fn = generate_position_distribution_uniform_box<Float>;
        gen_cfg.mass_fn     = generate_mass_distribution_uniform<Float>;

        std::vector<BodyT<Float>> bodies = generate_distribution(gen_cfg);

        Direct<Float>            direct({.bodies = bodies});
        BarnesHut<Float>::Config bh_cfg{};
        bh_cfg.bodies = bodies;
        bh_cfg.theta  = static_cast<Float>(0.5);
        BarnesHut<Float> bh(bh_cfg);

        BarnesHutLinear<Float>::Config bhl_cfg{};
        bhl_cfg.bodies        = bodies;
        bhl_cfg.theta         = static_cast<Float>(0.5);
        bhl_cfg.reserve_nodes = static_cast<USize>(n * 2);
        BarnesHutLinear<Float> bhl(bhl_cfg);

        Float dt = static_cast<Float>(0.0);

        direct.step(dt);
        bh.step(dt);
        bhl.step(dt);

        {
            std::string label = "direct N=" + std::to_string(n);
            bench.run(label.c_str(), [&] { direct.step(dt); });
        }

        {
            std::string label = "barnes_hut N=" + std::to_string(n);
            bench.run(label.c_str(), [&] { bh.step(dt); });
        }

        {
            std::string label = "barnes_hut_linear N=" + std::to_string(n);
            bench.run(label.c_str(), [&] { bhl.step(dt); });
        }

        std::cout << "Completed benchmarks for N = " << n << std::endl;
    }

    return 0;
}
