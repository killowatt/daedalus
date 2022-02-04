#pragma once

#include "SDL2/SDL.h"
#include "vulkan/vulkan.h" // we will keep these out of the global namespace later
#include "glm/glm.hpp"
#include "vk_mem_alloc.h"

#include <vector>
#include <iostream>
#include <array>

#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

class Engine
{
public:
	Engine();

	SDL_Window* Window = nullptr;

	uint32_t winWidth = 1280;
	uint32_t winHeight = 720;

	//VkInstance Instance = nullptr;
	//VkPhysicalDevice PhysicalDevice = nullptr;
	// VkDevice Device = nullptr;
	//VkSwapchainKHR Swapchain = nullptr;

	VulkanDevice* NewDevice;
	// VulkanSwapchain NewSwapChain;

	VmaAllocator Allocator;

	// uint32_t GraphicsFamily = 0;

	// VkQueue GraphicsQueue = nullptr;

	VkSurfaceKHR Surface = nullptr;

	//VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

	const uint32_t MAX_FRAMES_AHEAD = 2;
	// VkSemaphore imageAvailableSemaphores[MAX_FRAMES_AHEAD];

	// TODO: replace with statically allocated ^
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	uint32_t currentFrame = 0;



	VkBuffer VertexBuffer;
	VmaAllocation VertexBufferAllocation;
	VkBuffer IndexBuffer;
	VmaAllocation IndexBufferAllocation;

	

	//VkRenderPass RenderPass;
	VkPipelineLayout PipelineLayout;

	VkPipeline GraphicsPipeline;

	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.47f, 0.0f}},
		{{0.5f, -0.5f}, {1.0f, 0.45f, 0.0f}},
		{{0.5f, 0.5f}, {1.0f, 0.35f, 0.0f}},
		{{-0.5f, 0.5f}, {1.0f, 0.37f, 0.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

private:
	void Initialize();
	void Cleanup();

	void Render();

	void CreateInstance();
	void CreateSurface();
	void SelectPhysicalDevice();
	void CreateDevice();
	void CreateMemoryAllocator();

	// Swapchain
	void CreateSwapchain();
	void CreateImageViews();

	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateRenderPass();

	void CreateFramebuffers();

	void CreateCommandPool();
	void CreateCommandBuffers();

	void CreateSemaphores();

	void CreateVertexBuffer();
	void CreateIndexBuffer();

	void CleanupSwapchain();
	void RecreateSwapchain();

	VkShaderModule CreateShaderModule(const std::vector<char>& code);
};
