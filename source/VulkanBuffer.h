#pragma once

#include "VulkanDevice.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

enum class BufferType
{
	Vertex,
	Index
};

class VulkanBuffer
{
public:
	VkBuffer Buffer = VK_NULL_HANDLE;
	VmaAllocation Allocation = nullptr;

	static VulkanBuffer* Create(VulkanDevice* device, BufferType type, const void* data, size_t size);

private:
	VulkanBuffer();
};