#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "base/base.hpp"

namespace nbody {
class TriangleApplication {
   public:
    void run();

   private:
    void init();
    void init_glfw();
    void init_window();
    void init_vulkan();

    void run_main_loop();

    void cleanup();

   private:
    const I32 m_window_width  = 800;
    const I32 m_window_height = 600;

    GLFWwindow* m_window;
};
}  // namespace nbody
