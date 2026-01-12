#include <nanobench.h>

#include "base/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/preset.hpp"

using namespace nbody::sim;
using namespace nbody::base::type;

using Float = F64;
using Body  = BodyT<Float>;

void bench_barnes_hut(USize n, USize steps, Float theta, bool parallel) {
    std::vector<Body>        bodies = preset::benchmark::medium<Float>(n);
    BarnesHut<Float>::Config cfg = preset::config::barnes_hut_toy<Float>(bodies, parallel, theta);
    BarnesHut<Float>         sim(cfg);

    std::string name = "BarnesHut N=" + std::to_string(n) + " theta=" + std::to_string(theta) +
                       " parallel=" + std::to_string(parallel);
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
        for (Float theta : thetas) {
            for (USize i = 0; i < sizes.size(); ++i) {
                bench_barnes_hut(sizes[i], steps[i], theta, parallel);
            }
        }
    }
    return 0;
}
