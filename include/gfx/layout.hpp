// ==============================================================================
// layout.hpp
// Layout description utilities for UI elements in the nbody project.
// Provides the Layout enum for common anchor positions and helpers to check
// alignment (top, bottom, left, right).
// ==============================================================================

#pragma once

namespace nbody::gfx {

// ==============================================================================
// Layout Enum
// ==============================================================================

// Describes common anchor positions for UI layout.
enum class Layout {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
};

// ==============================================================================
// Layout Helper Functions
// ==============================================================================

// Returns true if layout is top-aligned.
bool layout_is_top(Layout layout);
// Returns true if layout is bottom-aligned.
bool layout_is_bottom(Layout layout);
// Returns true if layout is left-aligned.
bool layout_is_left(Layout layout);
// Returns true if layout is right-aligned.
bool layout_is_right(Layout layout);

}  // namespace nbody::gfx
