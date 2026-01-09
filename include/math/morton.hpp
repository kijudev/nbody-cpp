#pragma once

#include "base/type.hpp"

namespace nbody::math {
using namespace nbody::base::type;

namespace impl {
// NOTE: Does not use templates to ease leter intrisics optmization.

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

U32 morton_expand_bits_u16(U16 value);
U64 morton_expand_bits_u32(U32 value);

U32 morton_encode2_u32_f32(F32 x, F32 y, F32 min, F32 max);
U32 morton_encode2_u32_f64(F64 x, F64 y, F64 min, F64 max);
U64 morton_encode2_u64_f32(F32 x, F32 y, F32 min, F64 max);
U64 morton_encode2_u64_f64(F64 x, F64 y, F64 min, F64 max);
}  // namespace impl

template <FloatT Float, UintT Uint>
    requires(std::same_as<Uint, U32> || std::same_as<Uint, U64>)
Uint morton_encode2(Float x, Float y, Float min, Float max);
}  // namespace nbody::math
