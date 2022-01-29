#include "Engine.h"

#include "SDL2/SDL_vulkan.h"

Engine::Engine()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		return;
	}

	Window = SDL_CreateWindow(
		"Hello World",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720,
		SDL_WINDOW_VULKAN
	);

	if (Window == NULL)
	{
		return;
	}


	CreateInstance();
	CreateSurface();
	SelectPhysicalDevice();
	CreateDevice();
	CreateSwapChain();
	CreateImageViews();

	std::cout << "Main loop started\n";

	SDL_Event event;
	while (true)
	{
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				break;
			}
		}
	}

	for (auto imageView : SwapChainImageViews)
	{
		vkDestroyImageView(Device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(Device, SwapChain, nullptr);
	vkDestroyDevice(Device, nullptr);
	vkDestroySurfaceKHR(Instance, Surface, nullptr);
	vkDestroyInstance(Instance, nullptr);

	SDL_DestroyWindow(Window);
	SDL_Quit();
}

void Engine::Initialize()
{
}

void Engine::Cleanup()
{
}

void Engine::CreateInstance()
{
	// TODO: check what extensions are supported so we can return a list if we are missing one

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

void Engine::CreateSurface()
{
	SDL_Vulkan_CreateSurface(Window, Instance, &Surface);
}

void Engine::SelectPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

	std::cout << deviceCount << " compatible physical device(s)\n";
	for (VkPhysicalDevice device : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		std::cout << deviceProperties.deviceName << "\n";
	}

	PhysicalDevice = devices[0];
	std::cout << "First physical device selected\n";
}

void Engine::CreateDevice()
{
	// Queue Families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	bool foundFamily = false;
	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		const auto& family = queueFamilies[i];

		// TODO: advanced swapchain present queue selection logic
		VkBool32 supportsPresent = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &supportsPresent);

		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT && supportsPresent)
		{
			GraphicsFamily = i;
			foundFamily = true;

			break;
		}
	}
	if (!foundFamily)
	{
		std::cout << "no family found\n";
		return; // err
	}

	const float queuePriority = 1.0f;

	// TODO: for multiple queues, std::vector
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = GraphicsFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	// Device
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	VkResult result = vkCreateDevice(PhysicalDevice, &createInfo, nullptr, &Device);
	if (result != VK_SUCCESS)
	{
		// err
		std::cout << "vk create device failed\n";
	}

	vkGetDeviceQueue(Device, GraphicsFamily, 0, &GraphicsQueue);

}

void Engine::CreateSwapChain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, nullptr);

	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, availableFormats.data());

	VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	VkExtent2D extent = { 1280, 720 };

	uint32_t imageCount = capabilities.minImageCount + 1; // not mindful of max image count !

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(Device, &createInfo, nullptr, &SwapChain);
	if (result != VK_SUCCESS)
	{
		std::cout << "Swapchain failed to create\n";
		return;
	}

	vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, nullptr);

	SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, SwapChainImages.data());

	SwapChainImageFormat = surfaceFormat.format;
	SwapChainExtent = extent;
}

void Engine::CreateImageViews()
{
	SwapChainImageViews.resize(SwapChainImages.size());

	for (size_t i = 0; i < SwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = SwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = SwapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(Device, &createInfo, nullptr, &SwapChainImageViews[i]);
		if (result != VK_SUCCESS)
		{
			std::cout << "image view fail!\n";
			return;
		}
	}
}