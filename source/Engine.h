#pragma once

#include "SDL2/SDL.h"
#include "vulkan/vulkan.h" // we will keep these out of the global namespace later

#include <vector>
#include <iostream>

class Engine
{
public:
	Engine();

	SDL_Window* Window = nullptr;

	uint32_t winWidth = 1280;
	uint32_t winHeight = 720;

	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;
	VkSwapchainKHR Swapchain = nullptr;

	uint32_t GraphicsFamily = 0;

	VkQueue GraphicsQueue = nullptr;

	VkSurfaceKHR Surface = nullptr;

	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;

	std::vector<VkImageView> SwapChainImageViews;

	std::vector<VkFramebuffer> SwapChainFramebuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	uint32_t currentFrame = 0;

	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;


	VkRenderPass RenderPass;
	VkPipelineLayout PipelineLayout;

	VkPipeline GraphicsPipeline;

	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;


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

private:
	void Initialize();
	void Cleanup();

	void Render();

	void CreateInstance();
	void CreateSurface();
	void SelectPhysicalDevice();
	void CreateDevice();
	// Swapchain
	void CreateSwapchain();
	void CreateImageViews();

	void CreateGraphicsPipeline();
	void CreateRenderPass();

	void CreateFramebuffers();

	void CreateCommandPool();
	void CreateCommandBuffers();

	void CreateSemaphores();

	void CleanupSwapchain();
	void RecreateSwapchain();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);
};
