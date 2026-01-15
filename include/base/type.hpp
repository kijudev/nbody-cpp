// This header contains some of basic utilities:
// - Type aliases for basic data types conforming with the design principles of
// the project.
// - Macros for common operations.

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nbody::base::type {
// NOTE: Type aliases for basic data types conforming with the design principles
// of the project.
// WHY: I am just being pedantic about the naming conventions :)
using U8    = std::uint8_t;
using U16   = std::uint16_t;
using U32   = std::uint32_t;
using U64   = std::uint64_t;
using USize = std::size_t;
using I8    = std::int8_t;
using I16   = std::int16_t;
using I32   = std::int32_t;
using I64   = std::int64_t;
using F32   = float;
using F64   = double;

using Bytecode = std::vector<U8>;

// NOTE: Aliased floating point types.
template <typename Float>
concept FloatT = std::same_as<Float, float> || std::same_as<Float, double>;

template <typename Uint>
concept UintT = std::same_as<Uint, U8> || std::same_as<Uint, U16> ||
                std::same_as<Uint, U32> || std::same_as<Uint, U64>;

template <typename Int>
concept IntT = std::same_as<Int, I8> || std::same_as<Int, I16> ||
               std::same_as<Int, I32> || std::same_as<Int, I64>;

template <typename Number>
concept NumberT = FloatT<Number> || UintT<Number> || IntT<Number>;

template <typename Number>
concept SignedNumberT = FloatT<Number> || IntT<Number>;
}  // namespace nbody::base::type
