#include "math/morton.hpp"

#include <algorithm>
#include <limits>

#include "base/assert.hpp"
#include "base/type.hpp"

namespace nbody::math {
using namespace nbody::base::type;

namespace impl {
U32 morton_f32_to_u32_as_u16(F32 value, F32 min, F32 max) {
    F32 clamped    = std::clamp(value, min, max);
    F32 normalized = (clamped - min) / (max - min);
    return static_cast<U32>(normalized * std::numeric_limits<U16>::max());
}

U32 morton_f64_to_u32_as_u16(F64 value, F64 min, F64 max) {
    F64 clamped    = std::clamp(value, min, max);
    F64 normalized = (clamped - min) / (max - min);
    return static_cast<U32>(normalized * std::numeric_limits<U16>::max());
}

U64 morton_f32_to_u64_as_u32(F32 value, F32 min, F32 max) {
    F64 clamped = std::clamp(static_cast<F64>(value), static_cast<F64>(min), static_cast<F64>(max));
    F64 normalized =
        (clamped - static_cast<F64>(min)) / (static_cast<F64>(max) - static_cast<F64>(min));
    return static_cast<U64>(normalized * std::numeric_limits<U32>::max());
}

U64 morton_f64_to_u64_as_u32(F64 value, F64 min, F64 max) {
    F64 clamped    = std::clamp(value, min, max);
    F64 normalized = (clamped - min) / (max - min);
    return static_cast<U64>(normalized * std::numeric_limits<U32>::max());
}

U32 morton_expand_bits_u16_portable(U16 value) {
    U32 x = static_cast<U32>(value);

    x = (x | (x << 8)) & 0x00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F;
    x = (x | (x << 2)) & 0x33333333;
    x = (x | (x << 1)) & 0x55555555;

    return x;
}

U64 morton_expand_bits_u32_portable(U32 value) {
    uint64_t x = value;

    x = (x | (x << 16)) & 0x0000FFFF0000FFFF;
    x = (x | (x << 8)) & 0x00FF00FF00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0F;
    x = (x | (x << 2)) & 0x3333333333333333;
    x = (x | (x << 1)) & 0x5555555555555555;

    return x;
}

// TODO: Implement.
U32 morton_expand_bits_u16_intrisics(U16 value) {
    ASSERT(false, "TODO: Implement");
    (void)value;
    return 0;
}

// TODO: Implement.
U64 morton_expand_bits_u32_intrisics(U32 value) {
    ASSERT(false, "TODO: Implement");
    (void)value;
    return 0;
}

U32 morton_expand_bits_u16(U16 value) { return morton_expand_bits_u16_portable(value); }

U64 morton_expand_bits_u32(U32 value) { return morton_expand_bits_u32_portable(value); }

U32 morton_encode2_u32_f32(F32 x, F32 y, F32 min, F32 max) {
    U32 ix = morton_f32_to_u32_as_u16(x, min, max);
    U32 iy = morton_f32_to_u32_as_u16(y, min, max);
    return (morton_expand_bits_u16(iy) << 1) | morton_expand_bits_u16(ix);
}

U32 morton_encode2_u32_f64(F64 x, F64 y, F64 min, F64 max) {
    U32 ix = morton_f64_to_u32_as_u16(x, min, max);
    U32 iy = morton_f64_to_u32_as_u16(y, min, max);
    return (morton_expand_bits_u16(iy) << 1) | morton_expand_bits_u16(ix);
}

U64 morton_encode2_u64_f32(F32 x, F32 y, F32 min, F32 max) {
    U64 ix = morton_f32_to_u64_as_u32(x, min, max);
    U64 iy = morton_f32_to_u64_as_u32(y, min, max);
    return (morton_expand_bits_u32(iy) << 1) | morton_expand_bits_u32(ix);
}

U64 morton_encode2_u64_f64(F64 x, F64 y, F64 min, F64 max) {
    U64 ix = morton_f64_to_u64_as_u32(x, min, max);
    U64 iy = morton_f64_to_u64_as_u32(y, min, max);
    return (morton_expand_bits_u32(iy) << 1) | morton_expand_bits_u32(ix);
}

template <FloatT Float, UintT Uint>
Uint morton_encode2(Float x, Float y, Float min, Float max) {
    if constexpr (std::is_same_v<Uint, U32>) {
        if constexpr (std::is_same_v<Float, F32>) {
            return impl::morton_encode2_u32_f32(x, y, min, max);
        } else {
            return impl::morton_encode2_u32_f64(x, y, min, max);
        }
    } else {
        if constexpr (std::is_same_v<Float, F32>) {
            return impl::morton_encode2_u64_f32(x, y, min, max);
        } else {
            return impl::morton_encode2_u64_f64(x, y, min, max);
        }
    }
};

template U32 morton_encode2(F32 x, F32 y, F32 min, F32 max);
template U32 morton_encode2(F64 x, F64 y, F64 min, F64 max);
template U64 morton_encode2(F32 x, F32 y, F32 min, F32 max);
template U64 morton_encode2(F64 x, F64 y, F64 min, F64 max);
}  // namespace impl
}  // namespace nbody::math
