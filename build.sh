#!/usr/bin/env bash
set -e

PROFILE="debug"
OUT_DIR="build"

FLAGS_VERSION="-std=c++23"
FLAGS_DEBUG="-O1 -g -Wall -Wextra -Werror -Wpedantic -Isrc"
FLAGS_RELEASE="-O3 -g -Wall -Wextra -Werror -Wpedantic -Isrc"
FLAGS_DIST="-O3 -Wall -Wextra -Werror -Wpedantic -Isrc"
FLAGS_LIB_VULKAN="-lvulkan"
FLAGS_LIB_GLFW="-lglfw"

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
clang++ $FLAGS_VERSION apps/foo/main.cpp -o "$OUT_DIR/foo" $FLAGS_PROFILE
echo "apps/foo/main.cpp"

echo "--- Building Shaders ---"
# TODO: Implement shader compilation.

echo "--- Building Tests ---"
# TODO: Implement test compilation.

echo "--- Success ---"
