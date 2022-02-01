#pragma once

#include "vulkan/vulkan.h"

class VulkanDevice
{
public: // TODO:
	VulkanDevice();

public:
	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;
};