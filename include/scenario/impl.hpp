// ==============================================================================
// impl.hpp
// Scenario module internal helpers for UI, input, and simulation state.
// Provides time formatting, mouse picking, slingshot state, and related
// helpers.
// ==============================================================================

#pragma once

#include <optional>
#include <span>
#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "math/vec.hpp"
#include "sim/type.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

// ==============================================================================
// Time Formatting Helpers
// ==============================================================================

// Formats a time factor as a human-readable string (e.g., "2.5 years/sec").

template <FloatT Float>
std::string format_time(Float factor, Float year, Float day, Float hour,
                        Float minute);

template <FloatT Float>
std::optional<USize> get_body_at_mouse_position_au(
    gfx::Camera<Float>                                      camera,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies);
// Returns the index of the body under the mouse cursor in AU coordinates, if
// any.

template <FloatT Float>
struct SlingshotState {
    // State for interactive "slingshot" body launching in the UI.
    bool               is_active{false};       // Is the slingshot active?
    math::Vec2T<Float> start_pos{0.0, 0.0};    // Start position of drag
    math::Vec2T<Float> current_pos{0.0, 0.0};  // Current drag position
    Float              base_mass{1.0};         // Mass of the launched body
    Float              radius_scale{0.5};      // Visual radius scale
    Float              velocity_scale{1.0};    // Velocity scale factor
};

// Handles mouse input for slingshot state (drag to set velocity/direction).
template <FloatT Float>
void handle_slingshot_input(SlingshotState<Float>&    state,
                            const gfx::Camera<Float>& camera);

// Draws the slingshot UI overlay (drag line, preview, etc).
template <FloatT Float>
void draw_slingshot(const SlingshotState<Float>& state,
                    const gfx::Camera<Float>& camera, Float scale_factor);

// Creates a new body from the current slingshot state and camera.
template <FloatT Float>
sim::BodyT<Float> create_slingshot_body(const SlingshotState<Float>& state,
                                        const gfx::Camera<Float>&    camera);

}  // namespace nbody::scenario::impl
