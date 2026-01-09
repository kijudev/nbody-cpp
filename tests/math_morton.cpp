#include "base/type.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <set>
#include <vector>

#include "math/morton.hpp"

using namespace nbody::base::type;
using namespace nbody::math;

template <typename Float, typename Uint>
Uint encode2(Float x, Float y, Float min = 0.0, Float max = 1.0) {
    return morton_encode2<Float, Uint>(x, y, min, max);
}

// NOTE: Generate an NxN regular grid of Morton codes (u32, using F32 normalization).
static std::vector<U32> generate_morton_grid_u32(USize n, F32 min = 0.0f, F32 max = 1.0f) {
    std::vector<U32> codes;
    codes.reserve(n * n);

    for (USize j = 0; j < n; ++j) {
        for (USize i = 0; i < n; ++i) {
            const F32 x = (n == 1) ? 0.0f : static_cast<F32>(i) / static_cast<F32>(n - 1);
            const F32 y = (n == 1) ? 0.0f : static_cast<F32>(j) / static_cast<F32>(n - 1);
            codes.push_back(encode2<F32, U32>(x, y, min, max));
        }
    }

    return codes;
}

// NOTE: Generate an NxN regular grid of Morton codes (u64, using F64 normalization).
static std::vector<U64> generate_morton_grid_u64(USize n, F64 min = 0.0, F64 max = 1.0) {
    std::vector<U64> codes;
    codes.reserve(n * n);

    for (USize j = 0; j < n; ++j) {
        for (USize i = 0; i < n; ++i) {
            const F64 x = (n == 1) ? 0.0 : static_cast<F64>(i) / static_cast<F64>(n - 1);
            const F64 y = (n == 1) ? 0.0 : static_cast<F64>(j) / static_cast<F64>(n - 1);
            codes.push_back(encode2<F64, U64>(x, y, min, max));
        }
    }

    return codes;
}

TEST_CASE("Morton; U32, F32 - grid uniqueness and row monotonicity") {
    const USize      N     = 16;  // 16x16 grid
    std::vector<U32> codes = generate_morton_grid_u32(N);

    // NOTE: Uniqueness; all codes in the grid should be unique.
    std::set<U32> uniq(codes.begin(), codes.end());
    CHECK(uniq.size() == codes.size());

    // NOTE: Monotonicity along x for each fixed row: codes should be non-decreasing.
    for (USize row = 0; row < N; ++row) {
        for (USize col = 1; col < N; ++col) {
            const U32 prev = codes[row * N + (col - 1)];
            const U32 curr = codes[row * N + col];
            CHECK(curr >= prev);
        }
    }
}

TEST_CASE("Morton; U64, F64 - small grid sanity") {
    const USize      N     = 8;
    std::vector<U64> codes = generate_morton_grid_u64(N);

    // NOTE: Basic uniqueness check.
    std::set<U64> uniq(codes.begin(), codes.end());
    CHECK(uniq.size() == codes.size());

    // NOTE: Basic monotonicity along rows.
    for (USize row = 0; row < N; ++row) {
        for (USize col = 1; col < N; ++col) {
            const U64 prev = codes[row * N + (col - 1)];
            const U64 curr = codes[row * N + col];
            CHECK(curr >= prev);
        }
    }
}

TEST_CASE("Morton - deterministic behavior and simple inequalities") {
    const F32 minf = 0.0f;
    const F32 maxf = 1.0f;

    // NOTE: Origin should encode to zero.
    const U32 origin = encode2<F32, U32>(0.0f, 0.0f, minf, maxf);
    CHECK(origin == static_cast<U32>(0));

    // NOTE: Opposite corner should produce a non-zero code and differ from origin.
    const U32 corner = encode2<F32, U32>(1.0f, 1.0f, minf, maxf);
    CHECK(corner != static_cast<U32>(0));
    CHECK(corner != origin);

    // NOTE: Same input -> same output (determinism).
    const U32 a = encode2<F32, U32>(0.33f, 0.66f, minf, maxf);
    const U32 b = encode2<F32, U32>(0.33f, 0.66f, minf, maxf);
    CHECK(a == b);

    // NOTE: Different points generally map to different codes.
    const U32 p1 = encode2<F32, U32>(0.1f, 0.2f, minf, maxf);
    const U32 p2 = encode2<F32, U32>(0.4f, 0.7f, minf, maxf);
    CHECK(p1 != p2);

    // NOTE: Swapping coordinates should typically change the code.
    const U32 ab = encode2<F32, U32>(0.1f, 0.9f, minf, maxf);
    const U32 ba = encode2<F32, U32>(0.9f, 0.1f, minf, maxf);
    CHECK(ab != ba);
}
