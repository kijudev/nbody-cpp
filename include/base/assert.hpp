// This file contains basic assert macros. When an assertion fails the message is
// augmented with the source file and line number where the ASSERT was invoked.

#pragma once

#include <cstdlib>
#include <string>
#include <string_view>

#include "log.hpp"  // IWYU pragma: export

namespace nbody {
namespace impl {
std::string assert_format_message(std::string_view message, const char* file, int line);
}  // namespace impl
}  // namespace nbody

#ifndef NDEBUG
#define ASSERT(condition, message)                                                 \
    do {                                                                           \
        if (!(condition)) {                                                        \
            std::string _nbody_assert_msg =                                        \
                nbody::impl::assert_format_message((message), __FILE__, __LINE__); \
            nbody::Logger::log(nbody::LogLayer::ASSERT, nbody::LogSeverity::FATAL, \
                               _nbody_assert_msg);                                 \
            std::abort();                                                          \
        }                                                                          \
    } while (false)
#define PANIC(message)                                                                           \
    do {                                                                                         \
        std::string _nbody_panic_msg =                                                           \
            nbody::impl::assert_format_message((message), __FILE__, __LINE__);                   \
        nbody::Logger::log(nbody::LogLayer::PANIC, nbody::LogSeverity::FATAL, _nbody_panic_msg); \
        std::abort();                                                                            \
    } while (false)
#else
#define ASSERT(condition, message) ((void)0)
#define PANIC(message)             ((void)0)
#endif
