#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>

#include "base/base.hpp"

namespace nbody {
namespace impl {
struct QueueFamilyIndices {
    std::optional<U32> graphics_family;
    std::optional<U32> present_family;


    bool is_complete() const;
};

// NOTE: vk::SurfaceKHR is a handle, so no need to pass it by reference.
QueueFamilyIndices get_queue_family_indices(const vk::PhysicalDevice& device, vk::SurfaceKHR surface);

// NOTE: vk::SurfaceKHR is a handle, so no need to pass it by reference.
bool is_physical_device_suitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR surface);
}  // namespace impl
}  // namespace nbody
