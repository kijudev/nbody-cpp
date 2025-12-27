#pragma once

namespace nbody {
class TriangleApplication {
   public:
    void run();

   private:
    void init();
    void init_vulkan();
    void init_glfw();

    void run_main_loop();

    void cleanup();
};
}  // namespace nbody
