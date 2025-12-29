# WARNING: GLOB_RECURSE is okay for prototyping, specific list preferred for production.
# TODO: Replace GLOB_RECURSE with a specific list of source files for production.
file(GLOB_RECURSE SOURCES "*.cpp")

add_library(nbody_lib
    ${SOURCES}
)

target_include_directories(nbody_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(nbody_lib PUBLIC
    Vulkan::Vulkan
    glfw
    shaderc_shared
    raylib
)
