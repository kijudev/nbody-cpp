#include "base/assert.hpp"

#include <string>
#include <string_view>

#include "base/type.hpp"

namespace nbody::base::impl {
using namespace nbody::base::type;

std::string assert_format_message(std::string_view message, const char* file, I32 line) {
    std::string out;
    out.reserve(message.size() + 64);
    out.append(message);
    out.append(" [");
    out.append(file);
    out.append(":");
    out.append(std::to_string(line));
    out.append("]");
    return out;
}
}  // namespace nbody::base::impl
