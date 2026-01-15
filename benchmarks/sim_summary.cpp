#include <nanobench.h>

#include <iostream>
#include <string>
#include <vector>

#include "base/type.hpp"
#include "sim/barnes_hut.hpp"
#include "sim/barnes_hut_morton.hpp"
#include "sim/direct.hpp"
#include "sim/preset.hpp"

using namespace nbody::sim;
using namespace nbody::base::type;

using Float = F64;
using Body  = BodyT<Float>;

inline std::string bool_str(bool b) { return b ? "1" : "0"; }

template <typename Float>
void bench_direct(USize n, USize steps, bool parallel, ankerl::nanobench::Bench& bench) {
    std::vector<Body> bodies = preset::benchmark::medium<Float>(n);
    auto              cfg    = preset::config::direct_toy<Float>(bodies, parallel);
    Direct<Float>     sim(cfg);

    std::string name = "Direct N=" + std::to_string(n) + " parallel=" + bool_str(parallel);

    bench.run(name, [&] {
        for (USize i = 0; i < steps; ++i) {
            sim.step(static_cast<Float>(0.01));
        }
    });
}

template <typename Float>
void bench_barnes_hut(USize n, USize steps, Float theta, bool parallel,
                      ankerl::nanobench::Bench& bench) {
    std::vector<Body> bodies = preset::benchmark::medium<Float>(n);
    auto              cfg    = preset::config::barnes_hut_toy<Float>(bodies, parallel, theta);
    BarnesHut<Float>  sim(cfg);

    std::string name = "BarnesHut N=" + std::to_string(n) + " theta=" + std::to_string(theta) +
                       " parallel=" + bool_str(parallel);

    bench.run(name, [&] {
        for (USize i = 0; i < steps; ++i) {
            sim.step(static_cast<Float>(0.01));
        }
    });
}

template <typename Float>
void bench_barnes_hut_morton(USize n, USize steps, Float theta, bool parallel,
                             ankerl::nanobench::Bench& bench) {
    std::vector<Body> bodies = preset::benchmark::medium<Float>(n);
    auto              cfg = preset::config::barnes_hut_morton_toy<Float>(bodies, parallel, theta);
    BarnesHutMorton<Float> sim(cfg);

    std::string name = "BarnesHutMorton N=" + std::to_string(n) +
                       " theta=" + std::to_string(theta) + " parallel=" + bool_str(parallel);

    bench.run(name, [&] {
        for (USize i = 0; i < steps; ++i) {
            sim.step(static_cast<Float>(0.01));
        }
    });
}

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("N-Body Simulation Summary Benchmark");

    const std::vector<USize> sizes  = {10000, 50000};
    const std::vector<USize> steps  = {2, 2};
    const std::vector<Float> thetas = {0.5};

    for (size_t i = 0; i < sizes.size(); ++i) {
        if (i >= steps.size() || steps[i] == 0) {
            std::cout << "\nSETTINGS\nN = " << sizes[i] << ", steps = 0 (skipped)\n";
            continue;
        }
        std::cout << "\n==============================\n";
        std::cout << "SETTINGS: N=" << sizes[i] << ", steps=" << steps[i] << "\n";

        std::cout << "Direct:\n";
        for (bool parallel : {false, true}) {
            std::ostringstream label;
            label << "D N=" << sizes[i] << " P=" << (parallel ? "1" : "0");
            std::cout << "  " << label.str() << "\n";
            bench.run(label.str(),
                      [&] { bench_direct<Float>(sizes[i], steps[i], parallel, bench); });
        }
        std::cout << "\n";

        std::cout << "Barnes-Hut:\n";
        for (bool parallel : {false, true}) {
            for (Float theta : thetas) {
                std::ostringstream label;
                label << "BH N=" << sizes[i] << " T=" << theta << " P=" << (parallel ? "1" : "0");
                std::cout << "  " << label.str() << "\n";
                bench.run(label.str(), [&] {
                    bench_barnes_hut<Float>(sizes[i], steps[i], theta, parallel, bench);
                });
            }
        }
        std::cout << "\n";

        std::cout << "Barnes-Hut-Morton:\n";
        for (bool parallel : {false, true}) {
            for (Float theta : thetas) {
                std::ostringstream label;
                label << "BHM N=" << sizes[i] << " T=" << theta
                      << " P=" << (parallel ? "1" : "0");
                std::cout << "  " << label.str() << "\n";
                bench.run(label.str(), [&] {
                    bench_barnes_hut_morton<Float>(sizes[i], steps[i], theta, parallel, bench);
                });
            }
        }
        std::cout << "==============================\n";
    }
    return 0;
}