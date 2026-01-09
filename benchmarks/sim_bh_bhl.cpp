#include <nanobench.h>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "base/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/barnes_hut_linear.hpp"
#include "sim/generator.hpp"
#include "sim/type.hpp"

int main() {
    using namespace nbody::base::type;
    using namespace nbody::sim;
    using Float = F64;
    static constexpr Float DT = 1.0;

    ankerl::nanobench::Bench bench;
    bench.title("N-Body: BarnesHut vs BarnesHutLinear");
    bench.minEpochTime(std::chrono::milliseconds(50));
    bench.warmup(3);

    std::vector<USize> sizes = {128, 512, 2048, 4096, 8192, 16384, 32768, 65536, 131072};

    for (USize n : sizes) {
        std::vector<BodyT<Float>> bodies = generate_distribution(GenerateDistributionConfig<Float>{
            .n           = n,
            .min_mass    = 1.0,
            .max_mass    = 100.0,
            .radius      = 100.0,
            .position_fn = generate_position_distribution_uniform_box<Float>,
            .mass_fn     = generate_mass_distribution_uniform<Float>,
        });

        BarnesHut<Float> bh(BarnesHut<Float>::Config{
            .bodies = bodies,
            .theta  = 0.5,
        });

        BarnesHutLinear<Float> bhl(BarnesHutLinear<Float>::Config{
            .bodies = bodies,
            .theta  = 0.5,
            .reserve_nodes = n * 2,
        });

        bh.step(DT);
        bhl.step(DT);

        {
            std::string label = "BarnesHut N=" + std::to_string(n);
            bench.run(label.c_str(), [&] { bh.step(DT); });
        }

        {
            std::string label = "BarnesHutLinear N=" + std::to_string(n);
            bench.run(label.c_str(), [&] { bhl.step(DT); });
        }

        std::cout << "Completed benchmarks for N = " << n << std::endl;
    }

    return 0;
}
