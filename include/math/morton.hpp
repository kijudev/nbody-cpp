// ==============================================================================
// morton.hpp
// Flexible, templated 2D Morton encoder utilities for spatial indexing.
// Provides functions to encode 2D/3D coordinates into Morton codes (Z-order
// curve), which are useful for spatial partitioning (e.g., quadtrees). Includes
// helpers for bit expansion, normalization, and encoding for both float and
// integer types. See below for details on the bit layout and usage.
// ==============================================================================

#pragma once

#include "base/type.hpp"

namespace nbody::math {
using namespace nbody::base::type;

template <typename MortonCode>
concept MortonCodeT =
    std::same_as<MortonCode, U32> || std::same_as<MortonCode, U64>;

namespace impl {
// These functions turn floating point values into their normalized
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

// These functions expand bits. Put 0 next to every bit of the provided
// number.
U32 morton_expand_bits_u16(U16 value);
U64 morton_expand_bits_u32(U32 value);

// These functions encode a pair of floats (position) into their morton
// code.
U32 morton_encode2_u32_f32(F32 x, F32 y, F32 min, F32 max);
U32 morton_encode2_u32_f64(F64 x, F64 y, F64 min, F64 max);
U64 morton_encode2_u64_f32(F32 x, F32 y, F32 min, F32 max);
U64 morton_encode2_u64_f64(F64 x, F64 y, F64 min, F64 max);
}  // namespace impl

// Encodes a pair of floats (position) into a morton code.
// The first type parameter is the float type. The second is the size of
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
