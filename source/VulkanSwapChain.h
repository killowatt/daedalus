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

	VkQueue PresentQueue = VK_NULL_HANDLE;

	VkRenderPass RenderPass;

	uint32_t CurrentImage;

	std::vector<VkImage> SwapChainImages;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;

	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;

public:
	void Create(uint32_t width, uint32_t height);
	uint32_t NextImage(VkSemaphore semaphore);
	void Present(VkSemaphore waitSemaphore);
};