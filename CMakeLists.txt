cmake_minimum_required(VERSION 4.0)

project(nbody)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app)
target_sources(app
    PRIVATE
        src/main.cpp
)
