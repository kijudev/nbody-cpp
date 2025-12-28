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
    add_library(doctest::doctest ALIAS doctest)
    target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR})
endif()

file(GLOB TEST_SOURCES "*.cpp")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(test_${test_name} ${test_src})
    target_link_libraries(test_${test_name} PRIVATE nbody_lib doctest::doctest)
    add_test(NAME ${test_name} COMMAND test_${test_name})
endforeach()
