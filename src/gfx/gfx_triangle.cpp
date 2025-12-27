#pragma once

#include "gfx_triangle.hpp"

#include <GLFW/glfw3.h>

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
    // Initialize Vulkan
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
}  // namespace nbody
