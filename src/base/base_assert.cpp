#include "base_assert.hpp"

namespace nbody {
namespace impl {
std::string assert_format_message(std::string_view message, const char* file, int line) {
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
}  // namespace impl
}  // namespace nbody
