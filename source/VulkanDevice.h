#pragma once

#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "SDL2/SDL_video.h"

#include <vector>

class Engine;
class VulkanBuffer;

// We could abstract into a VulkanContext class that has an Instance and can handle multiple devices
class VulkanDevice
{
public:
	VulkanDevice();

	// TEMP: !!! TODO:
	int32_t windowWidth = 0;
	int32_t windowHeight = 0;

	static VkInstance Instance;

	SDL_Window* Window = nullptr;
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkDevice Device = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VmaAllocator Allocator = VK_NULL_HANDLE;
	VulkanSwapchain Swapchain;

	uint32_t CurrentFrame = 0;

	// Queues
	int32_t GraphicsFamily = -1;
	int32_t PresentFamily = -1;

	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	VkQueue TransferQueue = VK_NULL_HANDLE;

	// Command Buffers
	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;

	// Sync Primitives
	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> Fences;

	void Initialize(SDL_Window* window);

	void BeginFrame(VkBuffer Buffer, VkBuffer IndexBuffer, size_t indsiz, VulkanPipeline* pipe);
	void Present();

	void BindVertexBuffer(const VulkanBuffer* const buffer);
	void BindIndexBuffer(const VulkanBuffer* const buffer);
	void BindPipeline(const VulkanPipeline* const pipeline);

	void DrawIndexed(size_t count);

	void SetFramebuffer(); // TODO:

protected:
	const uint32_t MAX_FRAMES_AHEAD = 2;

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
	void CreateSyncPrimitives();
	void CreateCommandBuffers();
};