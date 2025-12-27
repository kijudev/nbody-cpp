#pragma once

#include "base_fs.hpp"

#include <filesystem>
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

std::vector<std::string> get_file_paths_in_directory(const std::string& directory) {
    std::vector<std::string> files;
    std::filesystem::directory_iterator dir(directory);

    for (const std::filesystem::directory_entry& entry : dir) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().string());
        }
    }

    return files;
}

}  // namespace nbody
