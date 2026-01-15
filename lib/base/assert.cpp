#include <string>
#include <string_view>

#include "base/assert.hpp"
#include "base/type.hpp"

namespace nbody::base::impl {
using namespace nbody::base::type;

std::string format_assert_message(std::string_view message, const char* file,
                                  I32 line) {
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
