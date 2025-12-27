#!/usr/bin/env bash
set -e

PROFILE="debug"
OUT_DIR="build"

FLAGS_VERSION="-std=c++23"

FLAGS_DEFINES="-DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS"
FLAGS_DEBUG="-O1 -g -Wall -Wextra -Werror -Wpedantic -Wno-missing-designated-field-initializers -Isrc"
FLAGS_RELEASE="-O3 -g -Wall -Wextra -Werror -Wpedantic -Wno-missing-designated-field-initializers -Isrc"
FLAGS_DIST="-O3 -g -Wall -Wextra -Werror -Wpedantic -Wno-missing-designated-field-initializers -Isrc"

FLAGS_LIB_VULKAN=$(pkg-config --libs vulkan)
FLAGS_LIB_GLFW=$(pkg-config --libs glfw3)
FLAGS_LIB_SHADERC=$(pkg-config --libs shaderc)
FLAGS_INCLUDE_DIRS=$(pkg-config --cflags vulkan glfw3 shaderc)

mkdir -p "$OUT_DIR"

if [ "$1" != "" ]; then
    PROFILE="$1"
fi

if [ "$PROFILE" == "debug" ]; then
    FLAGS_PROFILE="$FLAGS_DEBUG"
elif [ "$PROFILE" == "release" ]; then
    FLAGS_PROFILE="$FLAGS_RELEASE"
elif [ "$PROFILE" == "dist" ]; then
    FLAGS_PROFILE="$FLAGS_DIST"
else
    echo "Invalid profile: $PROFILE"
    exit 1
fi

echo "--- Building Apps ---"
echo "gfx_triangle: Compiling..."
clang++ $FLAGS_VERSION $FLAGS_DEFINES $FLAGS_INCLUDE_DIRS apps/gfx_triangle/main.cpp -o "$OUT_DIR/gfx_triangle" $FLAGS_PROFILE $FLAGS_LIB_GLFW $FLAGS_LIB_VULKAN
echo "gfx_triangle: Finished."

echo "gfx_shader_embed: Compiling..."
clang++ $FLAGS_VERSION $FLAGS_DEFINES $FLAGS_INCLUDE_DIRS apps/gfx_shader_embed/main.cpp -o "$OUT_DIR/gfx_shader_embed" $FLAGS_PROFILE $FLAGS_LIB_SHADERC
echo "gfx_shader_embed: Finished."

echo "--- Success ---"
