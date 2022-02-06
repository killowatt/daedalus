#pragma once

#include "SDL2/SDL.h"
#include "vulkan/vulkan.h" // we will keep these out of the global namespace later
#include "glm/glm.hpp"
#include "vk_mem_alloc.h"

#include <vector>
#include <iostream>
#include <array>

#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"

class Engine
{
public:
	Engine();

	SDL_Window* Window = nullptr;

	uint32_t winWidth = 1280;
	uint32_t winHeight = 720;

	VulkanDevice* NewDevice;

	VulkanBuffer* Vb;
	VulkanBuffer* Ib;
	
	VulkanShader* NewShader;

	VulkanPipeline* NewPipeline;

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
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

	struct CameraBuffer {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

private:
	void Initialize();
	void Cleanup();

	void Render();
};
