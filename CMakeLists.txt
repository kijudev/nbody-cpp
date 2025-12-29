cmake_minimum_required(VERSION 3.13)

project(nbody LANGUAGES CXX)

# NOTE: Default to Debug if not specified.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# NOTE: Standard Setup.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# NOTE: General Flags.
add_compile_options(
    -Wall -Wextra -Wpedantic -Wno-missing-designated-field-initializers
    -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS
)

# NOTE: Debug Configuration.
add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)

# NOTE: Production Configuration.
add_compile_options($<$<CONFIG:Production>:-O3>)
add_compile_options($<$<CONFIG:Production>:-flto=thin>)
add_compile_options($<$<CONFIG:Production>:-DNDEBUG>)
add_compile_options($<$<CONFIG:Production>:-DNLOG>)
add_link_options($<$<CONFIG:Production>:-flto=thin>)
add_link_options($<$<CONFIG:Production>:-s>)

find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(raylib REQUIRED)
pkg_check_modules(SHADERC REQUIRED shaderc) # Note: usually requires package name 'shaderc'

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(benchmarks)
