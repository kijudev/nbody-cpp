#include <nanobench.h>

#include <string>

#include "base/type.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/preset.hpp"

using namespace nbody::sim;
using namespace nbody::base::type;

using Float = F64;
using Body  = BodyT<Float>;

void bench_barnes_hut_morton(USize n, USize steps, Float theta, bool parallel, bool radix) {
    std::vector<Body> bodies = preset::benchmark::medium<Float>(n);
    auto              cfg = preset::config::barnes_hut_morton_toy<Float>(bodies, parallel, theta);
    cfg.radix             = radix;
    BarnesHutMorton<Float> sim(cfg);

    std::string name = "BarnesHutMorton N=" + std::to_string(n) +
                       " theta=" + std::to_string(theta) + " parallel=" + std::to_string(parallel) +
                       " radix=" + std::to_string(radix);

    ankerl::nanobench::Bench().run(name, [&] {
        for (USize i = 0; i < steps; ++i) {
            sim.step(0.01);
        }
    });
}

int main() {
    const std::vector<USize> sizes  = {100, 1000, 10000};
    const std::vector<USize> steps  = {10, 5, 1};
    const std::vector<Float> thetas = {0.3, 0.5, 1.0};

    for (bool parallel : {false, true}) {
        for (bool radix : {false, true}) {
            for (Float theta : thetas) {
                for (USize i = 0; i < sizes.size(); ++i) {
                    bench_barnes_hut_morton(sizes[i], steps[i], theta, parallel, radix);
                }
            }
        }
    }
    return 0;
}
