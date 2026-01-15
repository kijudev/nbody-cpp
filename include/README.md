# Include

Contains the header files all the modules in the project.
Most of the templated functions and classes are implement in `lib`.
Sometimes implicit template instanciation is impossible, those cases are implemented directly in the header file.

# Modules

## Base

- `assert.hpp`: Simple `ASSERT` and `PANIC` macros
- `log.hpp`: Minimal logging system
- `parallel.hpp`: Parallel iterator
- `radix.hpp`: Simple radix sort
- `type.hpp`: Common typedefs; **Used across the whole project**

## Math

- `collision.hpp`: Collision detection helpers
- `impl.hpp`: Utils for the module; **Not used outside the module**
- `morton.hpp`: Flexible, templated 2d morton encoder
- `vec.hpp`: Vec2, Vec3, Vec4 primitives

## Gfx

- `box.hpp`: UI primitive; **Used across the whole project**
- `camera.hpp`: Thin camera wraper
- `const.hpp`: Colors, Sizes
- `draw.hpp`: Drawing helpers
- `grid.hpp`: Layout system utilizing grids for positioning
- `layout.hpp`: Layout description
- `window.hpp`: Thin abstraction over Raylib's window

## Sim

- `barnes_hut.hpp`: Barnes-Hut implementation using linear Quad Tree
- `barnes_hut_morton.hpp`: Barnes Hut implementation utilizing Morton Codes to sort bodies with Radix sort in order to optimize tree construction
- `const.hpp`: Constants, inluding presets for scales (`scale_au`, `scale_toy`)
- `direct.hpp`: Direct numerical solution for the n-body problem
- `generator.hpp`: Generator for mass, position and velocity distributions
- `integrator.hpp`: Numerical integrators (`euler`, `euler semi symplectic`, `verlet - the bad one :(`, `verlet - the good one :)`)
- `kepler.hpp`: Analitycal solution for 2 bodies; smaller body orbiting the larger one (eg. Earth-Sol system)
- `preset.hpp`: Collection of pre-made configs for simulation and generators; to be used in testing and benchmarks
- `type.hpp`: Typedefs and data types used across simulations, generators and presets.

## Scenario

- `barnes_hut_grid.hpp`: Barnes-Hut with grid visualisation
- `barnes_hut_morton_plummer.hpp`: Barnes-Hut using Morton codes with a plummer distrubution; large number of bodies
- `barnes_hut_plummer.hpp`: Barnes-Hut with a plummer distribution; large number of bodies
- `draw.hpp`: Utils for drawing; **Not used outside of the module**
- `galaxy.hpp`: A simulation of a globular galaxy (tangetial velocities applied); using the standard linear tree Barnes-Hut
- `kepler_euler_verlet.hpp`: A comparison between the analitycal solution (Kepler)
- `type.hpp`:
