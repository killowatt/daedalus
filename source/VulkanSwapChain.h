#pragma once

#include <vector>
#include <cstdint>
#include "vulkan/vulkan.h"

class VulkanSwapchain
{
public: // TODO: ditto
	VulkanSwapchain();

public: // TODO: not public?
	class VulkanDevice* Device;

	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;

	VkRenderPass RenderPass;

	uint32_t CurrentImage;

	std::vector<VkImage> Images; // TODO: rename
	std::vector<VkImageView> ImageViews;
	std::vector<VkFramebuffer> Framebuffers;

	VkFormat ImageFormat;
	VkExtent2D Extent;

public:
	void Create(uint32_t width, uint32_t height);
	uint32_t NextImage(VkSemaphore semaphore);
	void Present(VkSemaphore waitSemaphore);
};