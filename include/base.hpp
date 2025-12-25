// This header contains all of the basic utilities
// Types, macros, logger etc.

#pragma once

#include <cstddef>
#include <cstdint>

namespace nbody {
// Type aliases for basic data types conforming with the design principles of the project
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
}  // namespace nbody
