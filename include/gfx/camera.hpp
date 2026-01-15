// ==============================================================================
// camera.hpp
// Camera abstraction for 2D world-to-screen and screen-to-world transformations.
// Provides movement, zoom, and viewport utilities for rendering and navigation.
// ==============================================================================

#pragma once

#include <tuple>

#include "base/type.hpp"
#include "gfx/point.hpp"
#include "gfx/window.hpp"
#include "math/vec.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// ==============================================================================
// Camera
// ==============================================================================
// Templated 2D camera for world <-> screen transformations and controls.
// Provides position, zoom, movement speed, and scaling speed.
// Includes helpers for coordinate conversion and viewport checks.
template <FloatT Float>
struct Camera {
    using Vec2 = math::Vec2T<Float>;

    I32   screen_width{Window::DEAFULT_WITDH};   // Screen width in pixels
    I32   screen_height{Window::DEAFULT_HEIGHT}; // Screen height in pixels
    Vec2  pos{0.0, 0.0};                         // Camera position in world coordinates
    Float zoom{1.0};                             // Zoom factor
    Float movement_speed{1.0};                   // Camera movement speed
    Float scaling_speed{1.0};                    // Camera zoom scaling speed

    // Convert world position to screen point (integer pixel coordinates)
    Point world_to_screen(const Vec2& world_pos) const;
    // Convert world position to screen position (float coordinates)
    Vec2  world_to_screen_vec(const Vec2& world_pos) const;
    // Convert screen point (pixel) to world coordinates
    Vec2  screen_to_world(Point point) const;

    // Get world-space extent of the current viewport (min, max)
    std::tuple<Vec2, Vec2> viewport_extent_world() const;
    // Check if a world position is inside the viewport
    bool                   is_pos_in_viewport(const Vec2& pos) const;
    // Check if a world-space circle is visible in the viewport
    bool is_circle_in_viewport(const Vec2& center, Float radius) const;

    // Handle camera movement controls (arrows/WSAD). Call every frame.
    void handle_controls_movement(Float dt);
    // Handle camera zoom controls (scrolling). Call every frame.
    void handle_controls_zoom(Float dt);
};

}  // namespace nbody::gfx
