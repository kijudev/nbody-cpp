#pragma once

#include "gfx_triangle.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <vulkan/vulkan.hpp>

#include "base/base.hpp"
#include "base/base_assert.hpp"
#include "gfx/gfx_impl.cpp"
#include "gfx/gfx_impl.hpp"
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
    create_surface();
    create_physical_device();
    create_device();
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

    vk::ApplicationInfo application_info{.pApplicationName   = "TRIANGLE",
                                         .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                         .pEngineName        = "NO_ENGINE",
                                         .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
                                         .apiVersion         = VK_API_VERSION_1_3};

    U32          glfw_extension_count = 0;
    const char** glfw_extensions      = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> instance_extensions(glfw_extensions,
                                                 glfw_extensions + glfw_extension_count);

    if (M_ENABLE_VALIDATION_LAYERS) {
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    vk::InstanceCreateInfo instance_create_info{
        .pApplicationInfo        = &application_info,
        .enabledExtensionCount   = static_cast<U32>(instance_extensions.size()),
        .ppEnabledExtensionNames = instance_extensions.data()};

    if (M_ENABLE_VALIDATION_LAYERS) {
        instance_create_info.enabledLayerCount   = static_cast<U32>(m_validation_layers.size());
        instance_create_info.ppEnabledLayerNames = m_validation_layers.data();
    }

    m_instance = vk::createInstanceUnique(instance_create_info);
}

void TriangleApplication::create_surface() {
    ASSERT(m_instance, "Instance not created.");
    ASSERT(m_window, "Window not created.");

    VkSurfaceKHR surface_c;
    VkResult     result = glfwCreateWindowSurface(m_instance.get(), m_window, nullptr, &surface_c);

    ASSERT(result == VK_SUCCESS, "Failed to create surface.");

    m_surface = vk::UniqueSurfaceKHR(surface_c, m_instance.get());
}

void TriangleApplication::create_physical_device() {
    ASSERT(m_surface, "Surface not created.");

    std::vector<vk::PhysicalDevice> physical_devices = m_instance->enumeratePhysicalDevices();
    ASSERT(!physical_devices.empty(), "No physical devices found.");

    // TODO: Implement a smarter selection algorithm
    auto it = std::remove_if(physical_devices.begin(), physical_devices.end(),
                             [&](const vk::PhysicalDevice& device) {
                                 return !impl::is_physical_device_suitable(device, m_surface.get());
                             });

    physical_devices.erase(it, physical_devices.end());
    ASSERT(!physical_devices.empty(), "No suitable physical devices found.");

    m_physical_device = physical_devices.front();
}

void TriangleApplication::create_device() {
    ASSERT(m_physical_device, "Physical device not created.");

    // WARNING: Assuming that graphics_family supports presentation (true for 99% of cases).
    // TODO: Implement a more robust solution for selecting the best queue family.

    impl::QueueFamilyIndices queue_family_indices =
        impl::get_queue_family_indices(m_physical_device, m_surface.get());

    auto present_support = m_physical_device.getSurfaceSupportKHR(
        queue_family_indices.graphics_family.value(), m_surface.get());

    ASSERT(present_support, "Presentation not supported.");

    F32 queue_priority = 1.0f;

    // WARNING Assuming that graphics_family supports presentation (true for 99% of cases).
    // TODO: Implement a more robust solution.
    std::array<vk::DeviceQueueCreateInfo, 1> queue_create_infos{
        vk::DeviceQueueCreateInfo{
                                  .queueFamilyIndex = queue_family_indices.graphics_family.value(),
                                  .queueCount       = 1,
                                  .pQueuePriorities = &queue_priority,
                                  },
    };

    // NOTE: Fill out if needed.
    vk::PhysicalDeviceFeatures device_features{};

    // NOTE:
    // - Swapchain extension; crucial for rendering.
    std::array<const char*, 1> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo device_create_info{
        .queueCreateInfoCount    = static_cast<U32>(queue_create_infos.size()),
        .pQueueCreateInfos       = queue_create_infos.data(),
        .enabledExtensionCount   = static_cast<U32>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures        = &device_features,
    };

    m_device         = m_physical_device.createDeviceUnique(device_create_info);
    m_graphics_queue = m_device->getQueue(queue_family_indices.graphics_family.value(), 0);
    m_present_queue  = m_device->getQueue(queue_family_indices.present_family.value(), 0);
}

}  // namespace nbody
