// ==============================================================================
// window.hpp
// Thin abstraction over Raylib's window for the nbody project.
// Provides a Window struct with initialization and resize handling.
// ==============================================================================

#pragma once

#include <string>

#include "base/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// ==============================================================================
// Window Struct
// ==============================================================================
// Represents the main application window and its properties.
// Provides methods for initialization and resize handling.
struct Window {
    // Default window width and height.
    static constexpr I32 DEAFULT_WITDH = 800;
    static constexpr I32 DEAFULT_HEIGHT = 800;

    I32         width{DEAFULT_WITDH};   // Window width in pixels
    I32         height{DEAFULT_HEIGHT}; // Window height in pixels
    std::string title{};                // Window title

    // Initialize the window with current properties.
    void init() const;

    // Handle window resize events.
    void handle_resize();
};
}  // namespace nbody::gfx
