#include <raylib.h>

#include "gfx/window.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

void Window::init() const {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title.c_str());
}

// TODO: Implement.
void Window::handle_resize() {
    width  = GetScreenWidth();
    height = GetScreenHeight();
}

}  // namespace nbody::gfx
