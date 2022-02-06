#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>

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

	static VulkanBuffer* Create(class VulkanDevice* device, BufferType type, const void* data, size_t size);

protected:
	void Init(VulkanDevice* device, BufferType type, const void* data, size_t size);

	VulkanBuffer();
};

enum class AttributeType : uint32_t
{
	Float2,
	Float3,
	UnsignedInt
};

struct VertexAttribute
{
	AttributeType Type;
	uint32_t Count;
	uint32_t Stride;
	uint32_t Offset;
};

class VulkanVertexBuffer : public VulkanBuffer
{
public:

	std::vector<VertexAttribute> Attributes;

	static VulkanVertexBuffer* Create(class VulkanDevice* device, const void* data, size_t size, std::vector<VertexAttribute> attributes);

private:
	VulkanVertexBuffer();
};
