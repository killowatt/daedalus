#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>

class Engine;

class VulkanDevice
{
public:
	VulkanDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	VkInstance Instance;

	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkDevice Device = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VmaAllocator Allocator = VK_NULL_HANDLE;

	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	VkQueue TransferQueue = VK_NULL_HANDLE;

	int32_t GraphicsFamily;
	int32_t PresentFamily;

	void Initialize();

protected:
	const std::vector<const char*> DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
};