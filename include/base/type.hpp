// This header contains some of basic utilities:
// - Type aliases for basic data types conforming with the design principles of the project.
// - Macros for common operations.

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace nbody::base::type {
// NOTE: Type aliases for basic data types conforming with the design principles of the project.
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
template <typename T>
concept FloatT = std::floating_point<T>;

template <typename T>
concept UintT = std::same_as<T, U8> || std::same_as<T, U16> || std::same_as<T, U32> ||
                std::same_as<T, U64>;
}  // namespace nbody::base::type
