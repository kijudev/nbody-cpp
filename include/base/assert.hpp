#pragma once

#include <cstdlib>
#include <string>
#include <string_view>

#include "assert.h"  // IWYU pragma: export
#include "log.hpp"   // IWYU pragma: export

namespace nbody::base {
using namespace nbody::base::type;

namespace impl {
std::string assert_format_message(std::string_view message, const char* file, int line);
}  // namespace impl
}  // namespace nbody::base

#if !defined(NDEBUG)
#define ASSERT(condition, message)                                                \
    do {                                                                          \
        if (!(condition)) {                                                       \
            std::string _nbody_assert_msg =                                       \
                base::impl::assert_format_message((message), __FILE__, __LINE__); \
            base::Logger::log(base::LogLayer::ASSERT, base::LogSeverity::FATAL,   \
                              _nbody_assert_msg);                                 \
            std::abort();                                                         \
        }                                                                         \
    } while (false)
#define STATIC_ASSERT(condition, message) static_assert((condition))
#define PANIC(message)                                                                        \
    do {                                                                                      \
        std::string _nbody_panic_msg =                                                        \
            base::impl::assert_format_message((message), __FILE__, __LINE__);                 \
        base::Logger::log(base::LogLayer::PANIC, base::LogSeverity::FATAL, _nbody_panic_msg); \
        std::abort();                                                                         \
    } while (false)
#else
#define ASSERT(condition, message)        ((void)0)
#define STATIC_ASSERT(condition, message) static_assert((condition))
#define PANIC(message)                    ((void)0)
#endif
