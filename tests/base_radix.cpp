#include "base/radix.hpp"
#include "base/type.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <random>
#include <string>
#include <vector>

using namespace nbody::base;
using namespace nbody::base::type;

template <typename It, typename Key>
bool is_sorted_by(It first, It last, Key key) {
    if (first == last) {
        return true;
    }

    auto prev = key(*first);
    for (++first; first != last; ++first) {
        if (prev > key(*first)) {
            return false;
        }

        prev = key(*first);
    }

    return true;
}

TEST_CASE("Radix sort: sorts unsigned integers (U32)") {
    std::vector<U32> data = {5, 3, 9, 1, 0, 8, 7, 2, 6, 4};

    radix_sort(data.begin(), data.end());
    CHECK(std::is_sorted(data.begin(), data.end()));
}

TEST_CASE("Radix sort: sorts signed integers (I32)") {
    std::vector<I32> data = {5, -3, 9, 1, 0, -8, 7, 2, -6, 4};

    radix_sort(data.begin(), data.end());
    CHECK(std::is_sorted(data.begin(), data.end()));
}

TEST_CASE("Radix sort: sorts large random U64") {
    std::mt19937_64  rng(42);
    std::vector<U64> data(1000);

    for (U64& n : data) {
        n = rng();
    }

    radix_sort(data.begin(), data.end());
    CHECK(std::is_sorted(data.begin(), data.end()));
}

TEST_CASE("Radix sort: sorts large random I64") {
    std::mt19937_64  rng(1337);
    std::vector<I64> data(1000);

    for (I64& n : data) {
        n = static_cast<I64>(rng()) - (1LL << 62);
    }

    radix_sort(data.begin(), data.end());
    CHECK(std::is_sorted(data.begin(), data.end()));
}

TEST_CASE("Radix sort: sorts structs by integer key (id)") {
    struct Person {
        I32         id;
        std::string name;
    };

    std::vector<Person> people = {
        {3, "Alice"},
        {1, "Bob"  },
        {2, "Carol"},
        {5, "Dave" },
        {4, "Eve"  }
    };

    radix_sort(people.begin(), people.end(), [](const Person& p) { return p.id; });
    CHECK(is_sorted_by(people.begin(), people.end(), [](const Person& p) { return p.id; }));
}

TEST_CASE("Radix sort: stable for equal keys") {
    struct Item {
        I32  key;
        char tag;
    };

    std::vector<Item> items = {
        {1, 'a'},
        {2, 'b'},
        {1, 'c'},
        {2, 'd'},
        {1, 'e'}
    };

    radix_sort(items.begin(), items.end(), [](const Item& i) { return i.key; });
    std::vector<char> tags;

    for (const Item& item : items) {
        if (item.key == 1) {
            tags.push_back(item.tag);
        }
    }

    CHECK(tags == std::vector<char>({'a', 'c', 'e'}));
}

TEST_CASE("Radix sort: works with empty and single-element ranges") {
    std::vector<I32> empty;

    radix_sort(empty.begin(), empty.end());
    CHECK(empty.empty());

    // WHY: THE MEANING OF LIFE, THE UNIVERSE AND EVERYTHING!!!
    std::vector<I32> one = {42};
    radix_sort(one.begin(), one.end());

    CHECK(one.size() == 1);
    CHECK(one[0] == 42);
}
