#include "VulkanBuffer.h"

#include "Common.h"
#include "VulkanDevice.h"

VulkanBuffer::VulkanBuffer()
{
}

VulkanBuffer* VulkanBuffer::Create(VulkanDevice* device, BufferType type, const void* data, size_t size)
{
	VulkanBuffer* buffer = new VulkanBuffer();
	buffer->Init(device, type, data, size);
	return buffer;
}

void VulkanBuffer::Init(VulkanDevice* device, BufferType type, const void* data, size_t size)
{
	VkBufferUsageFlags flags;
	switch (type)
	{
	case BufferType::Vertex:
		flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case BufferType::Index:
		flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	default:
		CRITICAL_ERROR("Invalid buffer type");
	}

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = flags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationInfo = {};
	allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	vmaCreateBuffer(device->Allocator, &bufferInfo, &allocationInfo, &Buffer, &Allocation, nullptr);

	void* destination;
	vmaMapMemory(device->Allocator, Allocation, &destination);
	memcpy(destination, data, (size_t)bufferInfo.size);
	vmaUnmapMemory(device->Allocator, Allocation);
}

VulkanVertexBuffer::VulkanVertexBuffer()
{
}

VulkanVertexBuffer* VulkanVertexBuffer::Create(VulkanDevice* device, const void* data, size_t size, std::vector<VertexAttribute> attributes)
{
	VulkanVertexBuffer* buffer = new VulkanVertexBuffer();
	buffer->Init(device, BufferType::Vertex, data, size);
	buffer->Attributes = attributes;

	return buffer;
}