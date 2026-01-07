#include <raylib.h>

#include "gfx/window.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

void Window::init() const { InitWindow(width, height, title.c_str()); }

// TODO: Implement.
void Window::handle_resize() {}

}  // namespace nbody::gfx
