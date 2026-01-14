#include <raylib.h>

#include "base/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// --- Warm Color Palette ---
constexpr Color RED_WARM    = {255, 99, 71, 255};   // Warm tomato red
constexpr Color ORANGE_WARM = {255, 160, 0, 255};   // Warm orange
constexpr Color YELLOW_WARM = {255, 215, 0, 255};   // Warm gold yellow
constexpr Color GREEN_WARM  = {154, 205, 50, 255};  // Warm yellow-green
constexpr Color BLUE_WARM   = {100, 149, 237, 255}; // Warm cornflower blue

// --- Pale Variants ---
constexpr Color RED_PALE    = {255, 180, 170, 255}; // Pale red
constexpr Color ORANGE_PALE = {255, 210, 160, 255}; // Pale orange
constexpr Color YELLOW_PALE = {255, 240, 180, 255}; // Pale yellow
constexpr Color GREEN_PALE  = {200, 230, 160, 255}; // Pale green
constexpr Color BLUE_PALE   = {180, 200, 240, 255}; // Pale blue

constexpr I32 XXS = 4;
constexpr I32 XS  = 8;
constexpr I32 S   = 12;
constexpr I32 M   = 16;
constexpr I32 L   = 24;
constexpr I32 XL  = 32;
constexpr I32 XXL = 48;
}  // namespace nbody::gfx
