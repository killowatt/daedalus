#pragma once

#include "vulkan/vulkan.h"

#include "VulkanShader.h"
#include "VulkanBuffer.h"

class VulkanPipeline
{
public:
	VkPipeline Pipeline;
	VkPipelineLayout PipelineLayout;

	static VulkanPipeline* Create(class VulkanDevice* device, const VulkanShader* const shader, std::vector<VertexAttribute> attributes, uint32_t vertexSize);

	// void SetShader(const VulkanShader* const shader);

	// void Compile();

protected:
	static VkShaderModule CreateShader(VkDevice device, std::vector<uint8_t> bytes);
};