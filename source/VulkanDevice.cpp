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
	CreateDevice();
	CreateSyncPrimitives();
	CreateCommandBuffers();

	// Swapchain
	SDL_Vulkan_GetDrawableSize(Window, &windowWidth, &windowHeight);

	Swapchain.Device = this;
	Swapchain.Surface = Surface;
	//Swapchain.PresentQueue = PresentQueue;
	Swapchain.Create(windowWidth, windowHeight);

	// Memory Allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	allocatorInfo.physicalDevice = PhysicalDevice;
	allocatorInfo.device = Device;
	allocatorInfo.instance = Instance;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &Allocator);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to create memory allocator");
}

void VulkanDevice::BeginFrame(VkBuffer Buffer, VkBuffer IndexBuffer, size_t indsiz, VulkanPipeline* pipe)
{
	vkWaitForFences(Device, 1, &Fences[CurrentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(Device, 1, &Fences[CurrentFrame]);

	uint32_t imageIndex = Swapchain.NextImage(ImageAvailableSemaphores[CurrentFrame]);

	// Main (Swapchain) Render Pass
	// Cmd buffer
	vkResetCommandBuffer(CommandBuffers[CurrentFrame], 0);

	VkCommandBufferBeginInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult result = vkBeginCommandBuffer(CommandBuffers[CurrentFrame], &bufferInfo);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to start command buffer recording");

	constexpr float gray = 16.0f / 255.0f;
	VkClearValue clearColor = { {{gray, gray, gray, 1.0f }} };

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = Swapchain.RenderPass;
	passInfo.framebuffer = Swapchain.Framebuffers[imageIndex];
	passInfo.renderArea.offset = { 0, 0 };
	passInfo.renderArea.extent = Swapchain.Extent;
	passInfo.clearValueCount = 1;
	passInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(CommandBuffers[CurrentFrame], &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(CommandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->Pipeline);
}

void VulkanDevice::Present()
{
	vkCmdEndRenderPass(CommandBuffers[CurrentFrame]);

	VkResult result = vkEndCommandBuffer(CommandBuffers[CurrentFrame]);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to end command buffer recording");

	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &stageFlags;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &ImageAvailableSemaphores[CurrentFrame];
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &RenderFinishedSemaphores[CurrentFrame];

    result = vkQueueSubmit(GraphicsQueue, 1, &submitInfo, Fences[CurrentFrame]);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Queue submission failed");

	Swapchain.Present(RenderFinishedSemaphores[CurrentFrame]);
	CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_AHEAD;
}

void VulkanDevice::BindVertexBuffer(const VulkanBuffer* const buffer)
{
	// TODO: No offset stuff yet
	VkDeviceSize offsets = { 0 };
	vkCmdBindVertexBuffers(CommandBuffers[CurrentFrame], 0, 1, &buffer->Buffer, &offsets);
}

void VulkanDevice::BindIndexBuffer(const VulkanBuffer* const buffer)
{
	// TODO: Support different index buffer data type (u16 and u32 mainly i guess)
	vkCmdBindIndexBuffer(CommandBuffers[CurrentFrame], buffer->Buffer, 0, VK_INDEX_TYPE_UINT16);
}

void VulkanDevice::BindPipeline(const VulkanPipeline* const pipeline)
{
	vkCmdBindPipeline(CommandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Pipeline);
}

void VulkanDevice::DrawIndexed(size_t size)
{
	// TODO: whole buffer size helper method doesnt take size
	vkCmdDrawIndexed(CommandBuffers[CurrentFrame], static_cast<uint32_t>(size), 1, 0, 0, 0);
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
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to create instance");
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

		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

		// Not very robust
		VkDeviceSize memorySize = memoryProperties.memoryHeaps[0].size;
		double size = (double)memorySize / 1024.0 / 1024.0 / 1024.0;

		LOG_VK("%s (%.2f GB)", deviceProperties.deviceName, size);
	}

	PhysicalDevice = devices[0];
	LOG_VK("First physical device selected");
}

void VulkanDevice::CreateDevice()
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
}

void VulkanDevice::CreateSyncPrimitives()
{
	ImageAvailableSemaphores.resize(MAX_FRAMES_AHEAD);
	RenderFinishedSemaphores.resize(MAX_FRAMES_AHEAD);
	Fences.resize(MAX_FRAMES_AHEAD);

	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Signaled on start

	for (uint32_t i = 0; i < MAX_FRAMES_AHEAD; i++)
	{
		// lol ignore result
		vkCreateSemaphore(Device, &createInfo, nullptr, &ImageAvailableSemaphores[i]);
		vkCreateSemaphore(Device, &createInfo, nullptr, &RenderFinishedSemaphores[i]);
		vkCreateFence(Device, &fenceInfo, nullptr, &Fences[i]);
	}
}

void VulkanDevice::CreateCommandBuffers()
{
	// Command Pool
	// TODO: consider transient command buffers?
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = GraphicsFamily;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult result = vkCreateCommandPool(Device, &createInfo, nullptr, &CommandPool);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to create command pool");

	// Command Buffers
	CommandBuffers.resize(MAX_FRAMES_AHEAD); // TODO: consider per swapchain image? why if we only have 2 frames max anyway

	VkCommandBufferAllocateInfo allocation = {};
	allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation.commandPool = CommandPool;
	allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	result = vkAllocateCommandBuffers(Device, &allocation, CommandBuffers.data());
	CRITICAL_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers");
}