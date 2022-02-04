#pragma once

#include "VulkanSwapChain.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "SDL2/SDL_video.h"

#include <vector>

class Engine;

// We could abstract into a VulkanContext class that has an Instance and can handle multiple devices
class VulkanDevice
{
public:
	VulkanDevice();

	static VkInstance Instance;

	SDL_Window* Window = nullptr;
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkDevice Device = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VmaAllocator Allocator = VK_NULL_HANDLE;

	VulkanSwapchain Swapchain;

	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	VkQueue TransferQueue = VK_NULL_HANDLE;

	int32_t GraphicsFamily = -1;
	int32_t PresentFamily = -1;

	void Initialize(SDL_Window* window);

	void Present();

protected:
	const std::vector<const char*> AdditionalExtensions =
	{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const std::vector<const char*> ValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void CreateInstance();

	void SelectDevice();
	void CreateDevice();
};