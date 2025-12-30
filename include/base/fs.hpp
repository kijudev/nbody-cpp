#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <vector>
#include "base/type.hpp"

namespace base {
// NOTE: Reads the content of the file as a vector of characters. If the file does not exist,
// returns nullopt.
std::optional<std::vector<char>> file_read_as_chars(const std::string& filename);

// NOTE: Reads the content of the file as a string. If the file does not exist, returns nullopt.
std::optional<std::string> file_read_as_string(const std::string& filename);

std::optional<Bytecode> file_read_as_bytecode(const std::string& filename);

// NOTE: Writes the content to the file. If the file already exists, it will be overwritten.
// WARNING: If operation fails, returns silently.
// TODO: Implement error handling.
void file_write(const std::string& filename, const std::string& content);

// NOTE: Lists the file paths in the directory. If the directory does not exist, returns nullopt.
std::optional<std::vector<std::string>> dir_list_file_paths(const std::string& dirname);

// NOTE: Lists the file names in the directory. If the directory does not exist, returns nullopt.
std::optional<std::vector<std::string>> dir_list_file_names(const std::string& dirname);

// NOTE: Creates a directory with the given name. If the directory already exists, does nothing.
void dir_make(const std::string& dirname);

// NOTE: If the filename has no extension, the second element of the tuple will be an empty string.
std::tuple<std::string, std::string> filename_split_by_extension(const std::string& filename);

}  // namespace nbody
