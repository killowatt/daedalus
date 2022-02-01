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

	uint32_t currentFrame = 0;
	uint32_t maxFramesInFlight = 2;

	std::vector<VkImage> SwapChainImages;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;

	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

public:
	void Create(uint32_t width, uint32_t height);
	void NextImage(VkSemaphore semaphore, uint32_t* imageIndex);
	void Present(uint32_t imageIndex, VkSemaphore semaphore);
};