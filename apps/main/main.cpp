#include <iostream>

#include "entity.hpp"
#include "entity_pool.hpp"

int main() {
    nbody::EntityPool pool;

    nbody::Entity a = pool.insert();
    nbody::Entity b = pool.insert();

    std::cout << pool.debug_format() << "\n";

    pool.destroy(a);

    std::cout << pool.debug_format() << "\n";

    pool.destroy(b);

    std::cout << pool.debug_format() << "\n";

    pool.insert();
    pool.insert();
    pool.insert();
    pool.insert();
    pool.insert();

    std::cout << pool.debug_format() << "\n";

    return 0;
}
