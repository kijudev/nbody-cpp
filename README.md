# NBody
**WARNING: Although the project is fully functional, many optimizations are still missing. Cross-compilation for Windows and macOS needs to be implemented. Currently, the performance of the Windows executable is horrendous, so please take this project with a grain of salt :)**
---
**NBody-cpp** is an interactive 2D N-body simulation written in modern C++ as a university project.
It features multiple algorithms for gravitational dynamics, including the Barnes-Hut algorithm, and provides real-time visualization using Raylib.


![galaxy simulation](./docs/ndboy-galaxy.png)

## Features

- Direct O(N²) and Barnes-Hut (O(N log N)) algorithms
- Barnes-Hut variant with Morton codes and radix sort
- Analytical two-body (Kepler) solution
- Interactive camera and UI (Raylib + Raygui)
- Multiple preset scenarios (galaxy, solar system, grid, globular cluster distribution)
- Configurable simulation parameters
- Benchmarks and tests

## Build & Run

**Dependencies:**

- **CMake** (>= 3.13)
- **C++23** compiler (MSVC, Clang, or GCC)
- **Raylib** (system or autofetched)
- **Raygui** (system or autofetched)
- **Doctest** (system or autofetched)
- **Nanobench** (system or autofetched)

**Quick start (Debug):**

```bash
cmake -B build
cmake --build build
./build/apps/nbody
```

**Production build (optimized):**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Production
cmake --build build
./build/apps/nbody
```

For more details, see the Polish documentation:
[`docs/dokumentacja.md`](docs/dokumentacja.md)

# Sources and References
1. https://arborjs.org/docs/barnes-hut
2. https://ned.ipac.caltech.edu/level5/Sept13/Silk/Silk2.html
3. https://ics.uci.edu/~ihler/papers/ihler_area.pdf
4. https://andyljones.com/pybbfmm/
5. https://ui.adsabs.harvard.edu/abs/1986Natur.324..446B/abstract
6. https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda
7. https://www.youtube.com/watch?v=nCg3aXn5F3M
8. https://www.algorithm-archive.org/contents/verlet_integration/verlet_integration.html
9. https://www.gorillasun.de/blog/euler-and-verlet-integration-for-particle-physics/

# Gallery

![galaxy simulation](./docs/ndboy-galaxy.png)
![galaxy simulation](./docs/nbody-barnes-hut-grid.png)
![galaxy simulation](./docs/nbody-barnes-hut-plummer.png)
![galaxy simulation](./docs/nbody-barnes-hut-morton-plummer-closeup.png)
![galaxy simulation](./docs/nbody-menu.png)
![galaxy simulation](./docs/nbody-kepler.png)
