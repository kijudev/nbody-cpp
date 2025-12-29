{
  description = "C++ DevShell";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
      };
      llvm = pkgs.llvmPackages;
    in
    {
      devShells.${system}.default =
        pkgs.mkShell.override
          {
            stdenv = llvm.libcxxStdenv;
          }
          {
            packages = with pkgs; [
              # Utilities
              nixd
              nil
              package-version-server

              # Cmake
              gnumake
              bear
              pkg-config
              cmake
              cmake-format
              neocmakelsp

              # C++
              llvm.lldb
              valgrind
              clang-tools
              cppcheck

              # Vulkan
              vulkan-headers
              vulkan-loader
              vulkan-validation-layers
              vulkan-tools

              # Raylib
              raylib

              # GLFW
              glfw

              # Shaders
              shaderc

              # Testing
              doctest

              # Benchmarking
              nanobench
            ];

            shellHook = ''
              echo "======== C++ DevShell ========"
            '';
          };

    };
}
