#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>

#include "base/base.hpp"

namespace nbody {
namespace impl {
struct QueueFamilyIndices {
    std::optional<U32> graphics_family;

    bool is_complete() const;
};

QueueFamilyIndices get_queue_family_indices(const vk::PhysicalDevice& device);
bool is_physical_device_suitable(const vk::PhysicalDevice& device);
}  // namespace impl
}  // namespace nbody
