#pragma once

#include "gfx_triangle.hpp"

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "base/base.hpp"
#include "vulkan/vulkan.hpp"

namespace nbody {
void TriangleApplication::run() {
    init();
    run_main_loop();
    cleanup();
}

void TriangleApplication::init() {
    init_glfw();
    init_window();
    init_vulkan();
}

void TriangleApplication::run_main_loop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void TriangleApplication::cleanup() {
    // NOTE: Wait for the GPU to finish operations before resources are released.
    if (m_device) {
        m_device->waitIdle();
    }

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void TriangleApplication::init_glfw() { glfwInit(); }

void TriangleApplication::init_window() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_window_width, m_window_height, "Triangle", nullptr, nullptr);
}

void TriangleApplication::init_vulkan() {
    create_instance();
    create_physical_device();
    create_device();
    create_queue();
}

bool TriangleApplication::check_validation_layer_support() const {
    std::vector<vk::LayerProperties> available_layers = vk::enumerateInstanceLayerProperties();

    for (const char* layer_name : m_validation_layers) {
        bool layer_found = false;

        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }

    return true;
}

void TriangleApplication::create_instance() {
    ASSERT(M_ENABLE_VALIDATION_LAYERS && check_validation_layer_support(),
           "Validation layers requested, but not available.");

    vk::ApplicationInfo app_info{.pApplicationName   = "Triangle Application",
                                 .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                 .pEngineName        = "No Engine",
                                 .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
                                 .apiVersion         = VK_API_VERSION_1_3};

    (void)app_info;
}
}  // namespace nbody
