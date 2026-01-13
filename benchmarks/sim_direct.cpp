#include <nanobench.h>

#include <string>
#include <vector>

#include "base/type.hpp"
#include "sim/direct.hpp"
#include "sim/preset.hpp"

using namespace nbody::sim;
using namespace nbody::base::type;

using Float = F64;
using Body  = BodyT<Float>;

void bench_direct(USize n, USize steps, bool parallel) {
    std::vector<Body> bodies = preset::benchmark::medium<Float>(n);
    auto              cfg    = preset::config::direct_toy<Float>(bodies, parallel);
    Direct<Float>     sim(cfg);

    std::string name = "Direct N=" + std::to_string(n) + " parallel=" + std::to_string(parallel);
    ankerl::nanobench::Bench().run(name, [&] {
        for (USize i = 0; i < steps; ++i) {
            sim.step(0.01);
        }
    });
}

int main() {
    for (bool parallel : {false, true}) {
        bench_direct(100, 10, parallel);
        bench_direct(500, 5, parallel);
    }

    return 0;
}
