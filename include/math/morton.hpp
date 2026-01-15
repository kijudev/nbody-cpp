// NOTE: Morton codes are a way to turn 2D or 3D coordinates into one number.
// This number has a special property: it divides the space into what is
// essencialy a nested quad tree in the following fashion:
//       ^ Y
//       |
//       |   +-------+-------+
//       |   |       |       |
//       |   |   2   |   3   |
//       |   |       |       |
//       |   +-------+-------+
//       |   |       |       |
//       |   |   0   |   1   |
//       |   |       |       |
//       |   +-------+-------+
//       +----------------------> X
// This bit ordering is represented in the bit represention of the morton code.

#pragma once

#include "base/type.hpp"

namespace nbody::math {
using namespace nbody::base::type;

// NOTE: Morton codes can represented only as 32 or 64 bit unsigned integers.
template <typename MortonCode>
concept MortonCodeT =
    std::same_as<MortonCode, U32> || std::same_as<MortonCode, U64>;

namespace impl {
// NOTE: These functions turn floating point values into their normalized
// version and convert them into unsigned ints.
U32 morton_f32_to_u32_as_u16(F32 value, F32 min, F32 max);
U32 morton_f64_to_u32_as_u16(F64 value, F64 min, F64 max);
U64 morton_f32_to_u64_as_u32(F32 value, F32 min, F32 max);
U64 morton_f64_to_u64_as_u32(F64 value, F64 min, F64 max);

U32 morton_expand_bits_u16_portable(U16 value);
U64 morton_expand_bits_u32_portable(U32 value);

// TODO: Implement.
U32 morton_expand_bits_u16_intrisics(U16 value);

// TODO: Implement.
U64 morton_expand_bits_u32_intrisics(U32 value);

// NOTE: These functions expands bits. Put 0 next to every bit of the provided
// number.
U32 morton_expand_bits_u16(U16 value);
U64 morton_expand_bits_u32(U32 value);

// NOTE: These functions encode a pair of floats (position) into their morton
// code.
U32 morton_encode2_u32_f32(F32 x, F32 y, F32 min, F32 max);
U32 morton_encode2_u32_f64(F64 x, F64 y, F64 min, F64 max);
U64 morton_encode2_u64_f32(F32 x, F32 y, F32 min, F32 max);
U64 morton_encode2_u64_f64(F64 x, F64 y, F64 min, F64 max);
}  // namespace impl

// NOTE: Encodes a pair of floats (position) into a morton code.
// NOTE: The first type parameter is the float type. The second is the size of
// the morton code to be outputet.
template <FloatT Float, UintT Uint>
    requires(std::same_as<Uint, U32> || std::same_as<Uint, U64>)
Uint encode_to_morton(Float x, Float y, Float min, Float max) {
    if constexpr (std::same_as<Uint, U32>) {
        if constexpr (std::same_as<Float, F32>) {
            return impl::morton_encode2_u32_f32(x, y, static_cast<F32>(min),
                                                static_cast<F32>(max));
        } else {
            return impl::morton_encode2_u32_f64(
                static_cast<F64>(x), static_cast<F64>(y), static_cast<F64>(min),
                static_cast<F64>(max));
        }
    } else {
        if constexpr (std::same_as<Float, F32>) {
            return impl::morton_encode2_u64_f32(x, y, static_cast<F32>(min),
                                                static_cast<F32>(max));
        } else {
            return impl::morton_encode2_u64_f64(
                static_cast<F64>(x), static_cast<F64>(y), static_cast<F64>(min),
                static_cast<F64>(max));
        }
    }
}
}  // namespace nbody::math
