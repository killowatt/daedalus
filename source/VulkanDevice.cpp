#include "VulkanDevice.h"

#include "Common.h"
#include "Engine.h"

#include <set>

VulkanDevice::VulkanDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	Instance = instance;
	PhysicalDevice = physicalDevice;
	Surface = surface;
}

void VulkanDevice::Initialize()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	LOG_VK("%d total queue families", queueFamilyCount);

	GraphicsFamily = -1;
	PresentFamily = -1;
	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		const auto& family = queueFamilies[i];

		bool isGraphicsFamily = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;

		VkBool32 supportsPresent = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &supportsPresent);

		if (isGraphicsFamily && supportsPresent == VK_TRUE)
		{
			GraphicsFamily = i;
			PresentFamily = i;
			break;
		}
		else if (isGraphicsFamily && GraphicsFamily == -1)
		{
			GraphicsFamily = i;
		}
		else if (supportsPresent && PresentFamily == -1)
		{
			PresentFamily = i;
		}
	}
	CRITICAL_ASSERT(GraphicsFamily != -1 && PresentFamily != -1, "Could not acquire queue families");

	const float queuePriority = 1.0f;

	std::set<int32_t> families = { GraphicsFamily, PresentFamily };
	std::vector<VkDeviceQueueCreateInfo> queueInfos;

	for (int32_t family : families)
	{
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = family;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;

		queueInfos.push_back(queueInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	deviceInfo.pQueueCreateInfos = queueInfos.data();
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(PhysicalDevice, &deviceInfo, nullptr, &Device);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan device");

	vkGetDeviceQueue(Device, GraphicsFamily, 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, PresentFamily, 0, &PresentQueue);

	// Memory Allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	allocatorInfo.physicalDevice = PhysicalDevice;
	allocatorInfo.device = Device;
	allocatorInfo.instance = Instance;

	vmaCreateAllocator(&allocatorInfo, &Allocator);
}

void VulkanDevice::Present()
{

}