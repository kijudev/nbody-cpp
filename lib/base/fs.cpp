#include "base/fs.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "base/type.hpp"

namespace nbody {
std::optional<std::vector<char>> file_read_as_chars(const std::string& filename) {
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

std::optional<std::string> file_read_as_string(const std::string& filename) {
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

void file_write(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        return;
    }

    file.write(content.data(), content.size());
    file.close();
}

std::optional<std::vector<std::string>> dir_list_file_paths(const std::string& dirname) {
    std::vector<std::string>            files;
    std::filesystem::directory_iterator diriter(dirname);

    if (!std::filesystem::exists(dirname)) {
        return std::nullopt;
    }

    for (const std::filesystem::directory_entry& entry : diriter) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().string());
        }
    }

    return files;
}

std::optional<std::vector<std::string>> dir_list_file_names(const std::string& dirname) {
    std::vector<std::string>            files;
    std::filesystem::directory_iterator diriter(dirname);

    if (!std::filesystem::exists(dirname)) {
        return std::nullopt;
    }

    for (const std::filesystem::directory_entry& entry : diriter) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().filename().string());
        }
    }

    return files;
}

void dir_create(const std::string& dirname) {
    if (!std::filesystem::exists(dirname)) {
        std::filesystem::create_directory(dirname);
    }
}

std::tuple<std::string, std::string> filename_split_by_extension(const std::string& filename) {
    USize pos = filename.find_last_of('.');

    if (pos == std::string::npos) {
        return std::make_tuple(filename, "");
    }

    return std::make_tuple(filename.substr(0, pos), filename.substr(pos + 1));
}
}  // namespace nbody
