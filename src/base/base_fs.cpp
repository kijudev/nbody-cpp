#pragma once

#include "base_fs.hpp"

#include <fstream>
#include <vector>

#include "base_type.hpp"

namespace nbody {
std::optional<std::vector<char>> read_file_as_chars(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return std::nullopt;
    }

    USize             size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

std::optional<std::string> read_file_as_string(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate);

    if (!file.is_open()) {
        return std::nullopt;
    }

    USize       size = file.tellg();
    std::string buffer(size, '\0');

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}
}  // namespace nbody
