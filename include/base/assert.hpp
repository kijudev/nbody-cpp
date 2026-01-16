// ==============================================================================
// assert.hpp
// Provides assertion and panic macros for runtime and compile-time checks.
// Includes formatted error messages and logging integration.
// ==============================================================================

#pragma once

#include <cstdlib>
#include <string>
#include <string_view>

#include "assert.h"  // IWYU pragma: export
#include "log.hpp"   // IWYU pragma: export

namespace nbody::base {
using namespace nbody::base::type;

namespace impl {
// Creates a formatted string representation of the assert message.
std::string format_assert_message(std::string_view message, const char* file,
                                  int line);
}  // namespace impl
}  // namespace nbody::base

#if !defined(NDEBUG)
// Runtime assert.
#define ASSERT(condition, message)                                             \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::string _nbody_assert_msg = base::impl::format_assert_message( \
                (message), __FILE__, __LINE__);                                \
            base::Logger::log(base::LogLayer::ASSERT,                          \
                              base::LogSeverity::FATAL, _nbody_assert_msg);    \
            std::abort();                                                      \
        }                                                                      \
    } while (false)

// Static assert.
#define STATIC_ASSERT(condition, message) static_assert((condition))

// Runtime assert that always fails.
#define PANIC(message)                                                        \
    do {                                                                      \
        std::string _nbody_panic_msg =                                        \
            base::impl::format_assert_message((message), __FILE__, __LINE__); \
        base::Logger::log(base::LogLayer::PANIC, base::LogSeverity::FATAL,    \
                          _nbody_panic_msg);                                  \
        std::abort();                                                         \
    } while (false)
#else
#define ASSERT(condition, message)        ((void)0)
#define STATIC_ASSERT(condition, message) static_assert((condition))
#define PANIC(message)                    ((void)0)
#endif
