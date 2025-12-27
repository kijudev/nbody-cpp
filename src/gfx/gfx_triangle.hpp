#pragma once

#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <vulkan/vulkan.hpp>

#include "base/base.hpp"

namespace nbody {
class TriangleApplication {
   public:
    void run();

   private:
    void init();
    void run_main_loop();
    void cleanup();

    void init_glfw();
    void init_window();
    void init_vulkan();

   private:
    const I32   m_window_width  = 800;
    const I32   m_window_height = 600;
    GLFWwindow* m_window{};

    const std::array<const char*, 1> m_validation_layers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    static constexpr bool M_ENABLE_VALIDATION_LAYERS = false;
#else
    static constexpr bool M_ENABLE_VALIDATION_LAYERS = true;
#endif

    // NOTE: vk::UniqueHandles are released automatically when the destructor is called; RAII.
    vk::UniqueInstance     m_instance{};
    vk::UniqueSurfaceKHR   m_surface{};
    vk::PhysicalDevice     m_physical_device{};  // NOTE: Not UniqueHandle; Owned by Instance.
    vk::UniqueDevice       m_device{};
    vk::Queue              m_graphics_queue{};  // NOTE: Not UniqueHandle; Owned by Device.
    vk::Queue              m_present_queue{};   // NOTE: Not UniqueHandle; Owned by Device.
    vk::UniqueSwapchainKHR m_swapchain{};
    std::vector<vk::Image> m_swapchain_images{};  // NOTE: Image handles are owned by the swapchain.
    vk::Format             m_swapchain_image_format{};
    vk::Extent2D           m_swapchain_extent{};

   private:
    bool check_validation_layer_support() const;

    void create_instance();
    void create_surface();
    void create_physical_device();
    void create_device();  // NOTE: Creates a logical device and queues.
    void create_swapchain();
};
}  // namespace nbody
