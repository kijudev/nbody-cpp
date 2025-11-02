# C++23 Minimal Template

A minimal, modern C++ project template designed for clarity and fast iteration using a robust set of LLVM tools and a Nix development environment.

---

## Project Structure

The project follows a standard structure to clearly separate different file types:

- `apps/` - Contains **main application entry points** (e.g., `apps/my_app/main.cpp`).
- `include/` - **Public header files** (`.hpp`) for the reusable library code.
- `lib/` - **Library source files** (`.cpp`) that implement the headers in `include/`.
- `tests/` - **Unit and integration test files** (using `doctest`).
- `bin/` - **Build artifacts** (executables, object files, etc.).

---

## Dependencies

This project is intended to be built within a **Nix development shell** (`nix develop`), which provides the necessary toolchain and utilities.

| Category      | Tool                        | Purpose                                                   |
| :------------ | :-------------------------- | :-------------------------------------------------------- |
| **Toolchain** | `clang++`, `lldb`, `libcxx` | C++23 compiler, debugger, and standard library.           |
| **Build**     | `gnumake`                   | Build system.                                             |
| **Tooling**   | `clangd`, `clang-format`    | Language Server Protocol (LSP) and code formatter.        |
| **Tooling**   | `bear`                      | Generates `compile_commands.json` for editor integration. |
| **Analysis**  | `cppcheck`, `valgrind`      | Static code analysis and dynamic memory debugger.         |
| **Testing**   | `doctest`                   | Lightweight unit testing framework.                       |

---

## Build and Run Commands

### Quick Start

The fastest way to get started is by entering the shell and building everything:

1.  **Enter the development shell:**

    ```bash
    nix develop
    ```

2.  **Build all code and run tests:**
    ```bash
    make all run-tests
    ```
    _(The `all` target builds all apps and tests.)_

### Make Commands

| Command                 | Description                                                                  |
| :---------------------- | :--------------------------------------------------------------------------- |
| `nix develop`           | Enters the Nix shell, making all dependencies available.                     |
| `make` / `make all`     | **Compiles all** applications and tests.                                     |
| `make apps`             | Compiles only the executables in `apps/`.                                    |
| `make tests`            | Compiles only the test executables in `bin/tests/`.                          |
| `make run-tests`        | Executes all compiled tests.                                                 |
| `make clean`            | Removes all build artifacts (`bin/` and `compile_commands.json`).            |
| `make compile-commands` | Generates `compile_commands.json` using `bear` for tooling (e.g., `clangd`). |
