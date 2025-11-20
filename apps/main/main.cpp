#include <iostream>

#include "entity.hpp"
#include "entity_pool.hpp"

int main() {
    nbody::EntityPool pool;

    nbody::Entity a = pool.insert();
    nbody::Entity b = pool.insert();
    nbody::Entity c = pool.insert();

    std::cout << "I\n";
    std::cout << a.debug_format() << "\n";
    std::cout << b.debug_format() << "\n";
    std::cout << c.debug_format() << "\n";

    pool.destroy(b);
    b = pool.get_by_index(b.index());

    std::cout << "II\n";
    std::cout << a.debug_format() << "\n";
    std::cout << b.debug_format() << "\n";
    std::cout << c.debug_format() << "\n";

    nbody::Entity d = pool.insert();

    std::cout << "III\n";
    std::cout << a.debug_format() << "\n";
    std::cout << d.debug_format() << "\n";
    std::cout << c.debug_format() << "\n";

    return 0;
}
