cmake_minimum_required(VERSION 3.5)

project(nbody LANGUAGES CXX)

# NOTE: Default to Debug.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# NOTE: Define Custom "Dist" Profile.
set(CMAKE_CXX_FLAGS_DIST "-Os -DNDEBUG" CACHE STRING "Flags used by the C++ compiler during distribution builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DIST "-s" CACHE STRING "Flags used for linking binaries during distribution builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DIST "-s" CACHE STRING "Flags used by the shared libraries linker during distribution builds." FORCE)

mark_as_advanced(CMAKE_CXX_FLAGS_DIST CMAKE_EXE_LINKER_FLAGS_DIST CMAKE_SHARED_LINKER_FLAGS_DIST)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# NOTE: Export compile_commands.json for Clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# NOTE: Compiler warnings; clang.
add_compile_options(
    -Wall -Wextra -Wpedantic -Wno-missing-designated-field-initializers
    -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS
)

# NOTE: Debug Flags (Split to prevent parsing errors)
add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)

# NOTE: Production Flags
add_compile_options($<$<CONFIG:Production>:-O3>)
add_compile_options($<$<CONFIG:Production>:-flto=thin>)

# NOTE: Dist Flags
add_compile_options($<$<CONFIG:Dist>:-flto=thin>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_link_options(-fsanitize=address,undefined)
endif()

# NOTE: Global Dependencies.
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(benchmarks)
