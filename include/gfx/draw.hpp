// ==============================================================================
// draw.hpp
// Drawing helpers for UI elements and text in the nbody project.
// Provides functions for drawing grids, text, and fitting text within boxes.
// ==============================================================================

#pragma once

#include <raylib.h>
#include <string>

#include "base/type.hpp"
#include "gfx/grid.hpp"
#include "gfx/layout.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

// ==============================================================================
// Drawing Functions
// ==============================================================================

// Draws a grid layout on the screen.
void draw_grid(const Grid& grid);

// Draws text inside a box at a given layout position and font size.
void draw_text(const Box& box, Layout layout, I32 size, const std::string& text,
               Color color = WHITE);

// Draws text fitted to the size of the box, adjusting font size as needed.
void draw_text_fit(const Box& box, Layout layout, const std::string& text,
                   Color color = WHITE);

}  // namespace nbody::gfx
