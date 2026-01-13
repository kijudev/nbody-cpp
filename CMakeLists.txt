cmake_minimum_required(VERSION 3.13)

project(nbody LANGUAGES CXX)

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

add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)

add_compile_options($<$<CONFIG:Production>:-O3>)
add_compile_options($<$<CONFIG:Production>:-DNDEBUG>)
add_compile_options($<$<CONFIG:Production>:-DNLOG>)
add_link_options($<$<CONFIG:Production>:-s>)

find_package(raylib REQUIRED)

find_path(raygui_SRC_DIR raygui.h PATH_SUFFIXES src)

if(NOT raygui_SRC_DIR)
    message(FATAL_ERROR "Could not find raygui.h in src")
endif()

include_directories(${raygui_SRC_DIR}/src)
include_directories(${raygui_SRC_DIR}/styles)

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(benchmarks)
