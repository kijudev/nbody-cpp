#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "add.hpp"
#include "doctest/doctest.h"

TEST_CASE("Testing the add function") {
    CHECK(add(0, 0) == 0);
    CHECK(add(1, 0) == 1);
    CHECK(add(0, 1) == 1);
    CHECK(add(-1, 1) == 0);
    CHECK(add(1, -1) == 0);
    CHECK(add(42, 69) == add(69, 42));
}
