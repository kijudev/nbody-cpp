# 2D n-body in C++ and Vulkan

# How to run the simulation

## Clone the repository

```bash
git clone https://github.com/username/nbody-cpp.git
```

## Windows

`todo`

## Linux

`todo`

## MacOS

`todo`

## Build the project

Debug; default.

```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++
ln -sf build/debug/compile_commands.json compile_commands.json # For Clangd
cmake --build build/debug
```

Production; optimized with debug information.

```bash
cmake -B build/prod -DCMAKE_BUILD_TYPE=Production -DCMAKE_CXX_COMPILER=clang++
cmake --build build/prod
```

Dist; small size, optimized for distribution, stripped.

```bash
cmake -B build/dist -DCMAKE_BUILD_TYPE=Dist -DCMAKE_CXX_COMPILER=clang++
cmake --build build/dist
```

## Run the simulation

```bash
./build/nbody
```

# Design Documentation

## Naming Conventions

- `snake_case` for variables and functions
- `m_snake_case` for private and protected member variables
- `impl_snake_case` for implementation details, helper methods
- `PascalCase` for classes, structs, enums
- `SCREAMING_SNAKE_CASE` for constants, macros, constexpr, enum members

## Comments

- `// NOTE:` general information, usage tips
- `// WARNING:` potential issues, limitations, or important information
- `// TODO:` tasks to be completed
- `// FIXME:` bugs to be fixed
- `// HACK:` implementation details, workarounds, or temporary solutions
- `// OPTIMIZE:` logic that works but can be improved for performance
- `// WHY:` reasons for certain design choices or implementation decisions

## Git

All the git commit messages must follow the convention: `<type>(<scope>): <subject>`.

### Commit types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test changes
- `chore`: Other changes
