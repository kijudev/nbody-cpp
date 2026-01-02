#include <vector>

#include "base/type.hpp"
#include "nbody2/const.hpp"
#include "nbody2/direct.hpp"
#include "nbody2/euler.hpp"
#include "nbody2/type.hpp"

int main() {
    std::vector<nbody2::BodyT<F32>> bodies;

    nbody2::BodyT<F32> a = {
        .pm  = {{10.0, 10.0}, 1.0},
        .vel = {0.0,          0.0},
        .acc = {0.0,          0.0},
    };

    nbody2::BodyT<F32> b = {
        .pm  = {{-10.0, -10.0}, 1.0},
        .vel = {0.0,            0.0},
        .acc = {0.0,            0.0},
    };

    nbody2::BodyT<F32> c = {
        .pm  = {{3.0, 3.0}, 1.0},
        .vel = {0.0,        0.0},
        .acc = {0.0,        0.0},
    };

    nbody2::SimDirect<F32>::Config config{
        .bodies = {a, b, c},
        .integrate_fn = nbody2::euler_integrate_body<F32>,
        .g = nbody2::G_TOY,
        .softening = nbody2::SOFTENING_TOY,
    };

    nbody2::SimDirect<F32> sim(config);

    return 0;
}
