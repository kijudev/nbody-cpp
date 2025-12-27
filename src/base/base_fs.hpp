#pragma once

#include <optional>
#include <string>
#include <vector>

namespace nbody {
std::optional<std::vector<char>> read_file_as_chars(const std::string& filename);
std::optional<std::string>       read_file_as_string(const std::string& filename);
}  // namespace nbody
