#pragma once

#include <raylib.h>

#include <string>

#include "base/type.hpp"
#include "gfx/grid.hpp"
#include "gfx/layout.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

void draw_grid(const Grid& grid);

void draw_text(const Box& box, Layout layout, I32 size, const std::string& text,
               Color color = WHITE);

void draw_text_fit(const Box& box, Layout layout, const std::string& text,
                   Color color = WHITE);

}  // namespace nbody::gfx
