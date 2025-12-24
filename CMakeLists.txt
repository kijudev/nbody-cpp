cmake_minimum_required(VERSION 3.22)

# Validate add_custom_command TARGET usage (avoid dev warning)
cmake_policy(SET CMP0175 NEW)

project(nbody VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Export compile commands for language tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Options
option(BUILD_APPS "Build executables found in apps/*" ON)
option(BUILD_TESTS "Build tests in tests/*" ON)
option(BUILD_BENCH "Build benchmarks in benchmarks/*" ON)
option(COMPILE_SHADERS "Compile shaders in shaders/* with glslc/glslang" ON)
option(COPY_COMPILE_COMMANDS "After generating compile_commands.json, copy it to the source root" OFF)
option(FAIL_ON_EMPTY_LIB "Fail configure step if there are no sources in lib/" OFF)

# Per-configuration compile options (applied to most targets via target_compile_options)
# Each generator-expression returns a semicolon-separated list of flags for the given config.
set(NBODY_COMMON_COMPILE_OPTIONS
  $<$<CONFIG:Debug>:-g;-O0;-DDEBUG;-Wall;-Wextra>
  $<$<CONFIG:RelWithDebInfo>:-O2;-g;-DNDEBUG>
  $<$<CONFIG:Release>:-O3;-DNDEBUG>
)

# Public include dir
set(NBODY_PUBLIC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# FetchContent is used later for dependencies
include(FetchContent)

# -----------------------
# Core library from lib/
# -----------------------
file(GLOB_RECURSE NBODY_LIB_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/lib/*.cpp"
)

if(NBODY_LIB_SOURCES)
    add_library(nbody STATIC ${NBODY_LIB_SOURCES})
    target_include_directories(nbody
    PUBLIC
      $<BUILD_INTERFACE:${NBODY_PUBLIC_INCLUDE_DIR}>
      $<INSTALL_INTERFACE:include>
  )
    target_compile_features(nbody PUBLIC cxx_std_23)
    # Apply common per-config compile options to the core library
    target_compile_options(nbody PRIVATE ${NBODY_COMMON_COMPILE_OPTIONS})
else()
    if(FAIL_ON_EMPTY_LIB)
        message(FATAL_ERROR "FAIL_ON_EMPTY_LIB is ON and no sources were found in lib/. Aborting configure.")
    else()
        # Provide a usable INTERFACE library so targets can still link to headers even when lib/ is empty
        add_library(nbody INTERFACE)
        target_include_directories(nbody INTERFACE $<BUILD_INTERFACE:${NBODY_PUBLIC_INCLUDE_DIR}>)
        message(STATUS "No sources in lib/. Created INTERFACE target `nbody`. Set FAIL_ON_EMPTY_LIB=ON to abort instead.")
    endif()
endif()

# -----------------------
# Nanobench
# -----------------------
if(BUILD_BENCH)
    FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG master
  )
    FetchContent_MakeAvailable(nanobench)
    # nanobench places headers in nanobench/include in the fetched source tree
    if(NOT TARGET nanobench_headers)
        add_library(nanobench_headers INTERFACE)
        target_include_directories(nanobench_headers INTERFACE ${nanobench_SOURCE_DIR}/include ${nanobench_SOURCE_DIR})
    endif()
    # Do not create namespaced alias targets; benchmarks will link to plain `nanobench_headers`
    message(STATUS "nanobench: fetched and INTERFACE target 'nanobench_headers' available")
endif()

# -----------------------
# Shader compilation
# -----------------------
if(COMPILE_SHADERS)
    find_program(GLSLC_EXECUTABLE glslc)
    find_program(GLSLANG_EXECUTABLE glslangValidator)

    file(GLOB_RECURSE SHADER_SRCS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_SOURCE_DIR}/shaders/*.comp"
    "${CMAKE_SOURCE_DIR}/shaders/*.geom"
    "${CMAKE_SOURCE_DIR}/shaders/*.glsl"
  )

    set(SPIRV_OUT_DIR "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${SPIRV_OUT_DIR}")

    set(SPIRV_FILES)
    foreach(SH ${SHADER_SRCS})
        file(RELATIVE_PATH REL "${CMAKE_SOURCE_DIR}/shaders" "${SH}")
        set(OUT "${SPIRV_OUT_DIR}/${REL}.spv")
        get_filename_component(OUTDIR "${OUT}" DIRECTORY)
        file(MAKE_DIRECTORY "${OUTDIR}")

        if(GLSLC_EXECUTABLE)
            add_custom_command(
        OUTPUT "${OUT}"
        COMMAND ${GLSLC_EXECUTABLE} "${SH}" -o "${OUT}"
        DEPENDS "${SH}"
        COMMENT "Compiling shader ${REL} -> ${OUT}"
        VERBATIM
      )
        elseif(GLSLANG_EXECUTABLE)
            add_custom_command(
        OUTPUT "${OUT}"
        COMMAND ${GLSLANG_EXECUTABLE} -V "${SH}" -o "${OUT}"
        DEPENDS "${SH}"
        COMMENT "Compiling shader ${REL} -> ${OUT}"
        VERBATIM
      )
        else()
            message(WARNING "No shader compiler found (glslc / glslangValidator). Skipping shader ${REL}.")
            continue()
        endif()

        list(APPEND SPIRV_FILES "${OUT}")
    endforeach()

    if(SPIRV_FILES)
        add_custom_target(shaders ALL DEPENDS ${SPIRV_FILES})
        message(STATUS "Shader target 'shaders' will compile ${SPIRV_FILES}")
    endif()
endif()

# -----------------------
# Vulkan
# -----------------------
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    message(STATUS "Found Vulkan")
else()
    message(STATUS "Vulkan not found (applications will build without linking Vulkan)")
endif()

# -----------------------
# GLFW3
# -----------------------
find_package(glfw3 QUIET)
if(glfw3_FOUND OR TARGET glfw)
    message(STATUS "Found GLFW")
else()
    message(STATUS "GLFW not found (applications will build without linking GLFW)")
endif()

# -----------------------
# Applications: apps/<name>/ -> executable <name>
# -----------------------
if(BUILD_APPS)
    file(GLOB APP_DIRS RELATIVE "${CMAKE_SOURCE_DIR}/apps" "${CMAKE_SOURCE_DIR}/apps/*")
    foreach(DIR ${APP_DIRS})
        set(APP_PATH "${CMAKE_SOURCE_DIR}/apps/${DIR}")
        if(NOT IS_DIRECTORY "${APP_PATH}")
            continue()
        endif()

        file(GLOB_RECURSE APP_SRCS CONFIGURE_DEPENDS "${APP_PATH}/*.cpp" "${APP_PATH}/*.cc" "${APP_PATH}/*.cxx")
        if(NOT APP_SRCS)
            message(STATUS "No sources in ${APP_PATH}; skipping app ${DIR}")
            continue()
        endif()

        add_executable(${DIR} ${APP_SRCS})
        target_include_directories(${DIR} PRIVATE ${NBODY_PUBLIC_INCLUDE_DIR} "${APP_PATH}")
        target_link_libraries(${DIR} PRIVATE nbody)
        target_compile_features(${DIR} PRIVATE cxx_std_23)
        # Apply common per-config compile options to application targets
        target_compile_options(${DIR} PRIVATE ${NBODY_COMMON_COMPILE_OPTIONS})

        # link optional graphics libs if found
        if(Vulkan_FOUND)
            if(TARGET Vulkan::Vulkan)
                target_link_libraries(${DIR} PRIVATE Vulkan::Vulkan)
            elseif(DEFINED Vulkan_LIBRARY)
                target_link_libraries(${DIR} PRIVATE ${Vulkan_LIBRARY})
            endif()
        endif()

        if(TARGET glfw)
            target_link_libraries(${DIR} PRIVATE glfw)
        elseif(glfw3_FOUND AND DEFINED GLFW_LIBRARIES)
            target_link_libraries(${DIR} PRIVATE ${GLFW_LIBRARIES})
        endif()

        # ensure shaders are compiled first when present
        if(TARGET shaders)
            add_dependencies(${DIR} shaders)
        endif()
    endforeach()
endif()

# -----------------------
# Tests: tests/*.cpp -> executable test_<name>
# -----------------------
if(BUILD_TESTS)
    include(CTest)
    enable_testing()

    # Prefer installed doctest; otherwise fetch single header
    find_package(doctest CONFIG QUIET)
    if(NOT doctest_FOUND)
        FetchContent_Declare(
      doctest_single
      URL https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h
    )
        FetchContent_MakeAvailable(doctest_single)
        set(DOCTEST_INCLUDE "${doctest_single_SOURCE_DIR}")
        if(NOT TARGET doctest_headers)
            add_library(doctest_headers INTERFACE)
            target_include_directories(doctest_headers INTERFACE ${DOCTEST_INCLUDE})
        endif()
        # Do not create a namespaced alias; tests will link to `doctest_headers` if the package isn't available
        message(STATUS "doctest: fetched single-header and INTERFACE target 'doctest_headers' available")
    else()
        message(STATUS "doctest: found package")
    endif()

    file(GLOB TEST_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp" "${CMAKE_SOURCE_DIR}/tests/*.cc")
    foreach(TF ${TEST_SRCS})
        get_filename_component(TNAME ${TF} NAME_WE)
        set(TGT test_${TNAME})
        add_executable(${TGT} ${TF})
        target_include_directories(${TGT} PRIVATE ${NBODY_PUBLIC_INCLUDE_DIR})
        if(TARGET doctest::doctest)
            target_link_libraries(${TGT} PRIVATE doctest::doctest)
        elseif(TARGET doctest_headers)
            target_link_libraries(${TGT} PRIVATE doctest_headers)
        endif()
        target_link_libraries(${TGT} PRIVATE nbody)
        target_compile_features(${TGT} PRIVATE cxx_std_23)
        # Apply common per-config compile options to test targets
        target_compile_options(${TGT} PRIVATE ${NBODY_COMMON_COMPILE_OPTIONS})
        add_test(NAME ${TGT} COMMAND ${TGT})
    endforeach()
endif()

# -----------------------
# Benchmarks: benchmarks/*.cpp -> executable bench_<name>
# -----------------------
if(BUILD_BENCH)
    file(GLOB BENCH_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/benchmarks/*.cpp" "${CMAKE_SOURCE_DIR}/benchmarks/*.cc")
    foreach(BF ${BENCH_SRCS})
        get_filename_component(BFN ${BF} NAME_WE)
        set(BT bench_${BFN})
        add_executable(${BT} ${BF})
        target_include_directories(${BT} PRIVATE ${NBODY_PUBLIC_INCLUDE_DIR})
        target_link_libraries(${BT} PRIVATE nbody)
        if(TARGET nanobench::nanobench)
            target_link_libraries(${BT} PRIVATE nanobench::nanobench)
        elseif(TARGET nanobench_headers)
            target_link_libraries(${BT} PRIVATE nanobench_headers)
        endif()
        target_compile_features(${BT} PRIVATE cxx_std_23)
        # Apply common per-config compile options to benchmark targets
        target_compile_options(${BT} PRIVATE ${NBODY_COMMON_COMPILE_OPTIONS})
    endforeach()
endif()

# -----------------------
# Bear helper: generate compile_commands.json
# -----------------------
find_program(BEAR_EXECUTABLE NAMES bear)
set(GEN_CC_TARGET_NAME generate-compile-commands)

if(BEAR_EXECUTABLE)
    # Prepare command list; run bear and optionally copy the compile database back to the source root.
    set(BEAR_CMDS
    COMMAND ${CMAKE_COMMAND} -E echo "Running bear to capture build and produce compile_commands.json..."
    COMMAND ${BEAR_EXECUTABLE} -- ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
  )
    if(COPY_COMPILE_COMMANDS)
        list(APPEND BEAR_CMDS COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json)
    endif()

    add_custom_target(${GEN_CC_TARGET_NAME}
    ${BEAR_CMDS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating compile_commands.json using bear"
  )
    message(STATUS "Found bear: ${BEAR_EXECUTABLE} -> target '${GEN_CC_TARGET_NAME}' available")
else()
    add_custom_target(${GEN_CC_TARGET_NAME}
    COMMAND ${CMAKE_COMMAND} -E echo "bear not found; install bear or run 'bear -- <build command>' manually to generate compile_commands.json"
    COMMENT "Bear not found: generate-compile-commands is a no-op"
  )
    message(STATUS "bear not found; 'generate-compile-commands' target is a no-op")
endif()

# -----------------------
# Summary
# -----------------------
message(STATUS "Configuration summary:")
message(STATUS "  lib sources:       ${NBODY_LIB_SOURCES}")
message(STATUS "  apps scanned:      ${APP_DIRS}")
message(STATUS "  tests scanned:     ${TEST_SRCS}")
message(STATUS "  benchmarks scanned: ${BENCH_SRCS}")
message(STATUS "  shaders output dir: ${SPIRV_OUT_DIR}")
message(STATUS "  copy compile db:   ${COPY_COMPILE_COMMANDS}")
message(STATUS "  fail on empty lib: ${FAIL_ON_EMPTY_LIB}")
