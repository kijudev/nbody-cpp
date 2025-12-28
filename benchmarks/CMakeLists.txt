include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG v4.3.11
)
FetchContent_MakeAvailable(nanobench)

file(GLOB BENCH_SOURCES "*.cpp")

foreach(bench_src ${BENCH_SOURCES})
    get_filename_component(bench_name ${bench_src} NAME_WE)
    add_executable(bench_${bench_name} ${bench_src})
    target_link_libraries(bench_${bench_name} PRIVATE nbody_lib nanobench::nanobench)
endforeach()
