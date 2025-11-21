#include <iostream>

#include "entity.hpp"
#include "entity_pool.hpp"

int main() {
    nbody::EntityPool pool;

    nbody::Entity a = pool.create();
    nbody::Entity b = pool.create();

    std::cout << pool.debug_format() << "\n";

    pool.destroy(a);

    std::cout << pool.debug_format() << "\n";

    pool.destroy(b);

    std::cout << pool.debug_format() << "\n";

    pool.create();
    pool.create();
    pool.create();
    pool.create();
    pool.create();

    std::cout << pool.debug_format() << "\n";

    return 0;
}
