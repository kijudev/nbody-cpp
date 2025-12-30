#include <vector>

#include "base/type.hpp"
#include "nbody2/barnes_hut.hpp"
#include "nbody2/const.hpp"
#include "nbody2/euler.hpp"
#include "nbody2/type.hpp"

int main() {
    std::vector<nbody2::BodyT<F32>> bodies;

    nbody2::BodyT<F32> a = {
        .pos = {1.0f, 1.0f},
          .vel = {0.0f, 0.0f},
          .acc = {0.0f, 0.0f},
          .mass = 1.0f
    };

    nbody2::BodyT<F32> b = {
        .pos = {-1.0f, -1.0f},
          .vel = {0.0f,  0.0f },
          .acc = {0.0f,  0.0f },
          .mass = 1.0f
    };

    nbody2::SimBarnesHut<F32> sim({a, b}, nbody2::euler_integrate_body<F32>, nbody2::G_TOY,
                                  nbody2::SOFTENING_TOY);

    sim.step(1.0);

    return 0;
}
