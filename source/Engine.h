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

	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;
	VkSwapchainKHR SwapChain = nullptr;

	uint32_t GraphicsFamily = 0;

	VkQueue GraphicsQueue = nullptr;

	VkSurfaceKHR Surface = nullptr;

	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;

	std::vector<VkImageView> SwapChainImageViews;

	VkRenderPass RenderPass;
	VkPipelineLayout PipelineLayout;

	VkPipeline GraphicsPipeline;

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

	void CreateInstance();
	void CreateSurface();
	void SelectPhysicalDevice();
	void CreateDevice();
	// Swapchain
	void CreateSwapChain();
	void CreateImageViews();

	void CreateGraphicsPipeline();
	void CreateRenderPass();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);
};
