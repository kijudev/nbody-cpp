cmake_minimum_required(VERSION 3.13)

project(nbody LANGUAGES CXX)

# Default to Debug if not specified.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO: Implement flags for other compilers.
add_compile_options(
    -Wall -Wextra -Wpedantic -Wno-missing-designated-field-initializers
    -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS
)

# Debug Configuration.
add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)

# Production Configuration.
add_compile_options($<$<CONFIG:Production>:-O3>)
add_compile_options($<$<CONFIG:Production>:-DNDEBUG>)
add_compile_options($<$<CONFIG:Production>:-DNLOG>)
add_link_options($<$<CONFIG:Production>:-s>)

# Dependencies.
find_package(raylib REQUIRED)

# Directories.
add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(benchmarks)
