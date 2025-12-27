#pragma once

#include "gfx_triangle.hpp"

#include <GLFW/glfw3.h>

namespace nbody {
void TriangleApplication::run() {
    init();
    run_main_loop();
}

void TriangleApplication::init() {
    init_glfw();
    init_window();
    init_vulkan();
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

void TriangleApplication::run_main_loop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void TriangleApplication::cleanup() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

}  // namespace nbody
