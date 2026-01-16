# NoBodyCares

**NBody-cpp** is a fast, interactive 2D n-body simulation written in modern C++.
It features multiple algorithms for gravitational dynamics, including a highly efficient Barnes-Hut implementation, and provides real-time visualization using Raylib.

## Features

- Direct O(N²) and Barnes-Hut (O(N log N)) algorithms
- Barnes-Hut variant with Morton codes and radix sort
- Analytical two-body (Kepler) solution
- Interactive camera and UI (Raylib + Raygui)
- Dynamic body addition ("slingshot" mode)
- Multiple preset scenarios (galaxy, solar system, grid, ring, etc.)
- Configurable simulation parameters
- Benchmark and test modes

## Build & Run

**Dependencies:**

- CMake (>= 3.13)
- C++23 compiler (MSVC, Clang, or GCC)
- Raylib and Raygui (system or auto-fetched)

**Quick start (Debug):**

```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
./build/nbody
```

**Production build (optimized):**

```bash
cmake -B build/prod -DCMAKE_BUILD_TYPE=Production
cmake --build build/prod
./build/prod/nbody
```

For more details, see the Polish documentation:
[`docs/dokumentacja.md`](docs/dokumentacja.md)
