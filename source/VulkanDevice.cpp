#include "VulkanDevice.h"

#include "Common.h"
#include "Engine.h"

#include "SDL2/SDL_vulkan.h"

#include <set>

VkInstance VulkanDevice::Instance = VK_NULL_HANDLE;

VulkanDevice::VulkanDevice()
{
}

void VulkanDevice::Initialize(SDL_Window* window)
{
	Window = window;

	CRITICAL_ASSERT(Window != nullptr, "No window provided");

	CreateInstance();
	SDL_Vulkan_CreateSurface(Window, Instance, &Surface);
	SelectDevice();

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

	// Swapchain
	int32_t windowWidth = 0;
	int32_t windowHeight = 0;
	SDL_GetWindowSize(Window, &windowWidth, &windowHeight);

	Swapchain.Device = this;
	Swapchain.Surface = Surface;
	Swapchain.PresentQueue = PresentQueue;
	Swapchain.Create(windowWidth, windowHeight);
}

void VulkanDevice::Present()
{

}

void VulkanDevice::CreateInstance()
{
	// TODO: check what extensions are supported so we can return a list if we are missing one
	if (Instance != VK_NULL_HANDLE)
		return; // Instance already created

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Daedalus";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t extensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(Window, &extensionCount, nullptr);

	std::vector<const char*> extensions(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(Window, &extensionCount, extensions.data());

	for (const char* extension : AdditionalExtensions)
	{
		extensions.push_back(extension);
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	createInfo.ppEnabledLayerNames = ValidationLayers.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
	if (result != VK_SUCCESS)
	{
		std::cout << "FAIL\n";
		return;
	}
}

void VulkanDevice::SelectDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

	LOG_VK("%d compatible physical device(s)", deviceCount);
	for (VkPhysicalDevice device : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		std::cout << deviceProperties.deviceName << "\n";
	}

	PhysicalDevice = devices[0];
	LOG_VK("First physical device selected");
}

void VulkanDevice::CreateDevice()
{

}