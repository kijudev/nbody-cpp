#include "gfx/window.hpp"

#include <raylib.h>

namespace nbody::gfx {
using namespace nbody::base::type;

void Window::init() const {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(width, height, title.c_str());
}

// TODO: Implement.
void Window::handle_resize() {
    width = GetScreenWidth();
    height = GetScreenHeight();
}

}  // namespace nbody::gfx
