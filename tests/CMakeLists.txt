cmake_policy(SET CMP0169 OLD)

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_GetProperties(doctest)
if(NOT doctest_POPULATED)
    FetchContent_Populate(doctest)
    add_library(doctest INTERFACE)
    target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR})
    add_library(doctest::doctest ALIAS doctest)
endif()

file(GLOB TEST_SOURCES "*.cpp")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(test_${test_name} ${test_src})
    target_link_libraries(test_${test_name} PRIVATE nbody_lib doctest::doctest)
    add_test(NAME ${test_name} COMMAND test_${test_name})
endforeach()
