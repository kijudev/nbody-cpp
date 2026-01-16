# ==============================================================================
#
# CMakeLists.txt - Main configuration file for the nbody-cpp project
#
# This file supports automatic fetching of Raylib/Raygui (FetchContent); fallbacks to using
# system-installed versions, and sets appropriate compiler flags for MSVC, GCC, and Clang.
#
# ==============================================================================

cmake_minimum_required(VERSION 3.13)

project(nbody LANGUAGES CXX)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# 1. Fetch Dependencies FIRST (so global flags don't mess them up)
# ==============================================================================

include(FetchContent)

# Raylib
set(RAYLIB_FOUND FALSE)
find_package(raylib QUIET)
if(raylib_FOUND)
    message(STATUS "Using system raylib")
else()
    message(STATUS "Fetching raylib via FetchContent")
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Raygui
set(RAYGUI_FOUND FALSE)
find_package(raygui QUIET)
if(raygui_FOUND)
    message(STATUS "Using system raygui")
    set(raygui_SRC_DIR ${raygui_DIR})
else()
    message(STATUS "Fetching raygui via FetchContent")
    FetchContent_Declare(
        raygui
        GIT_REPOSITORY https://github.com/raysan5/raygui.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(raygui)
    set(raygui_SRC_DIR ${raygui_SOURCE_DIR})
endif()

include_directories(${raygui_SRC_DIR}/src)
include_directories(${raygui_SRC_DIR}/styles)

# ==============================================================================
# 2. Define Your Subprojects
# ==============================================================================

add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(benchmarks)

# ==============================================================================
# 3. Apply Compiler Flags (Use target_compile_options generally, 
#    but if you stick to add_compile_options, do it HERE, after includes)
# ==============================================================================

if(MSVC)
    # Only apply these to C++ targets if possible, or handle carefully
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    
    # Split flags to avoid the "space in generator expression" error
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    add_compile_options($<$<CONFIG:Debug>:/Zi>)
    
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Release>:/DNDEBUG>)
else()
    # GCC/Clang options...
    add_compile_options(-Wall -Wextra -Wpedantic -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS)
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-missing-designated-field-initializers)
    endif()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wno-missing-field-initializers)
    endif()

    add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_compile_options($<$<CONFIG:Production>:-O3>)
    add_compile_options($<$<CONFIG:Production>:-DNDEBUG>)
    add_compile_options($<$<CONFIG:Production>:-DNLOG>)
    add_link_options($<$<CONFIG:Production>:-s>)
endif()