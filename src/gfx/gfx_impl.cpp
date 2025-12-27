#pragma once

#include "gfx_impl.hpp"

#include <vulkan/vulkan.hpp>

#include "base/base.hpp"

namespace nbody {
namespace impl {
bool QueueFamilyIndices::is_complete() const {
    return graphics_family.has_value() && present_family.has_value();
}

QueueFamilyIndices get_queue_family_indices(const vk::PhysicalDevice& device,
                                            const vk::SurfaceKHR      surface) {
    QueueFamilyIndices                     indices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    U32 i = 0;
    for (const vk::QueueFamilyProperties& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        if (device.getSurfaceSupportKHR(i, surface)) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        ++i;
    }

    return indices;
}

bool is_physical_device_suitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR surface) {
    QueueFamilyIndices indices = get_queue_family_indices(device, surface);
    return indices.is_complete();
}
}  // namespace impl
}  // namespace nbody
