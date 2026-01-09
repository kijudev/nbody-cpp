#include "base/type.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cmath>
#include <string>

#include "math/vec.hpp"

using namespace nbody::base::type;
using namespace nbody::math;

TEST_CASE("Vec2F32 - construction and basic arithmetic") {
    using Vec2 = Vec2T<F32>;

    Vec2 a = {1.0, 2.0};
    Vec2 b = {3.0, 4.0};

    SUBCASE("add") {
        Vec2 sum = a.add(b);
        CHECK(sum.is_approx_equal({4.0f, 6.0}));
    }

    SUBCASE("sub") {
        Vec2 diff = b.sub(a);
        CHECK(diff.is_approx_equal({2.0, 2.0}));
    }

    SUBCASE("mul elementwise") {
        Vec2 prod = a.mul(b);
        CHECK(prod.is_approx_equal({3.0, 8.0}));
    }

    SUBCASE("scale") {
        Vec2 s = a.scale(2.0f);
        CHECK(s.is_approx_equal({2.0, 4.0}));
    }
}

TEST_CASE("Vec2F32 - dot, length and length_sq") {
    using Vec2 = Vec2T<F32>;

    Vec2 a = {2.0, 3.0};
    Vec2 b = {4.0f, -1.0};

    SUBCASE("dot product") {
        F32 d = a.dot(b);
        CHECK(d == doctest::Approx(2.0f * 4.0f + 3.0f * -1.0f));
    }

    SUBCASE("length squared and length") {
        F32 len_sq = a.length_sq();
        CHECK(len_sq == doctest::Approx(13.0f));
        CHECK(a.length() == doctest::Approx(std::sqrt(13.0f)));
    }
}

TEST_CASE("Vec2F32 - normalization and zero handling") {
    using Vec2 = Vec2T<F32>;

    SUBCASE("normalize non-zero vector") {
        Vec2 a  = {3.0, 4.0};
        Vec2 an = a.normalized();
        CHECK(an.length() == doctest::Approx(1.0f));

        // NOTE: ensure direction preserved (unit vector close to (0.6, 0.8)).
        CHECK(an.is_approx_equal({0.6f, 0.8f}, 1e-6f));
    }

    SUBCASE("normalize zero vector returns zero") {
        Vec2 z  = {0.0f, 0.0f};
        Vec2 zn = z.normalized();
        CHECK(zn.is_approx_equal(z));
        CHECK(zn.length() == doctest::Approx(0.0f));
    }
}

TEST_CASE("Vec2F64 - distance, distance_sq and approx equality") {
    using Vec2 = Vec2T<F64>;

    Vec2 p1{0.0, 0.0};
    Vec2 p2{3.0, 4.0};

    SUBCASE("distance squared") { CHECK(p1.distance_sq(p2) == doctest::Approx(25.0)); }

    SUBCASE("distance") { CHECK(p1.distance(p2) == doctest::Approx(5.0)); }

    SUBCASE("approx equality with eps") {
        Vec2 a{1.000000000000001, 1.0};
        Vec2 b{1.0, 1.0};
        CHECK(a.is_approx_equal(b, 1e-12));
    }
}

TEST_CASE("Vec2 - to_string and as_raylib_vector") {
    using Vec2 = Vec2T<F32>;

    Vec2 a = {1.2345678f, -9.8765432f};

    SUBCASE("to_string non-empty and formatted") {
        std::string s = a.to_string();
        CHECK(!s.empty());
        CHECK(s.front() == '(');
        CHECK(s.back() == ')');
    }

    SUBCASE("as_raylib_vector preserves components within float precision") {
        auto rv = a.as_raylib_vector();
        CHECK(rv.x == doctest::Approx(a.x));
        CHECK(rv.y == doctest::Approx(a.y));
    }
}
