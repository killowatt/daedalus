#include "VulkanBuffer.h"
#include "Common.h"

VulkanBuffer::VulkanBuffer()
{
}

VulkanBuffer* VulkanBuffer::Create(VulkanDevice* device, BufferType type, const void* datazz, size_t size)
{
	VulkanBuffer* buffer = new VulkanBuffer();

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

	vmaCreateBuffer(device->Allocator, &bufferInfo, &allocationInfo, &buffer->Buffer, &buffer->Allocation, nullptr);

	void* destination;
	vmaMapMemory(device->Allocator, buffer->Allocation, &destination);
	memcpy(destination, datazz, (size_t)bufferInfo.size);
	vmaUnmapMemory(device->Allocator,  buffer->Allocation);

	return buffer;
}