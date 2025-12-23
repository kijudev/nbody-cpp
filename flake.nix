{
  description = "C++ DevShell";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
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

              # GLFW
              glfw

              # Shaders
              shaderc

              # Testing
              doctest

              # Benchmarking
              nanobench

              # Fonts
              freetype

              # Math
              glm
            ];

            shellHook = ''
              echo "======== C++ DevShell ========"
            '';
          };

    };
}
