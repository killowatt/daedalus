#include "Engine.h"

#include "SDL2/SDL_vulkan.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "File.h"
#include "Common.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

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

	if (Window == nullptr)
	{
		return;
	}

	// CreateInstance();
	// CreateSurface();
	// SelectPhysicalDevice();

	//NewDevice = new VulkanDevice(Instance, PhysicalDevice, Surface);
	NewDevice = new VulkanDevice();
	NewDevice->Initialize(Window);
	Allocator = NewDevice->Allocator; // too lazy

	//CreateDevice();
	//CreateMemoryAllocator();

	//NewSwapChain.Device = NewDevice;
	//NewSwapChain.PresentQueue = NewDevice->PresentQueue;
	//NewSwapChain.Surface = Surface;
	//NewSwapChain.Create(1280, 720);
	//CreateSwapchain();
	//CreateImageViews();

	//CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	//CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateCommandBuffers();
	CreateSemaphores();

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
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					std::cout << "resize \n";
					winWidth = event.window.data1;
					winHeight = event.window.data2;
				}
			}
		}

		Render();
	}

	vkDeviceWaitIdle(NewDevice->Device);

	Cleanup();

	SDL_DestroyWindow(Window);
	SDL_Quit();
}

void Engine::Initialize()
{
}

void Engine::Cleanup()
{
	CleanupSwapchain();

	vmaDestroyBuffer(Allocator, IndexBuffer, IndexBufferAllocation);
	vmaDestroyBuffer(Allocator, VertexBuffer, VertexBufferAllocation);
	vmaDestroyAllocator(Allocator);

	for (uint32_t i = 0; i < MAX_FRAMES_AHEAD; i++)
	{
		vkDestroySemaphore(NewDevice->Device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(NewDevice->Device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(NewDevice->Device, inFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(NewDevice->Device, CommandPool, nullptr);
	//for (auto framebuffer : SwapChainFramebuffers)
	//{
	//	vkDestroyFramebuffer(Device, framebuffer, nullptr);
	//}
	vkDestroyPipeline(NewDevice->Device, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(NewDevice->Device, PipelineLayout, nullptr);
	//vkDestroyRenderPass(Device, RenderPass, nullptr);
	//for (auto imageView : SwapChainImageViews)
	//{
	//	vkDestroyImageView(Device, imageView, nullptr);
	//}
	//vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	// vkDestroyDevice(Device, nullptr);
	//vkDestroySurfaceKHR(Instance, Surface, nullptr);
	//vkDestroyInstance(Instance, nullptr);

}


void Engine::CleanupSwapchain()
{
	//for (auto framebuffer : SwapChainFramebuffers)
	//{
	//	vkDestroyFramebuffer(Device, framebuffer, nullptr);
	//}

	vkFreeCommandBuffers(NewDevice->Device, CommandPool, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());

	vkDestroyPipeline(NewDevice->Device, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(NewDevice->Device, PipelineLayout, nullptr);

	//vkDestroyRenderPass(Device, RenderPass, nullptr);
	//for (auto imageView : SwapChainImageViews)
	//{
	//	vkDestroyImageView(Device, imageView, nullptr);
	//}
	//vkDestroySwapchainKHR(Device, Swapchain, nullptr);
}

void Engine::Render()
{
	vkWaitForFences(NewDevice->Device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(NewDevice->Device, 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex = NewDevice->Swapchain.NextImage(imageAvailableSemaphores[currentFrame]);

	VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &stageFlags;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	VkResult result = vkQueueSubmit(NewDevice->GraphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
	CRITICAL_ASSERT(result == VK_SUCCESS, "queue submit fail");

	// Presentation
	NewDevice->Swapchain.Present(renderFinishedSemaphores[currentFrame]);
	currentFrame = (currentFrame + 1) % MAX_FRAMES_AHEAD;
}

void Engine::CreateInstance()
{
	// TODO: check what extensions are supported so we can return a list if we are missing one

	//VkApplicationInfo applicationInfo = {};
	//applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	//applicationInfo.pApplicationName = "Daedalus";
	//applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	//applicationInfo.apiVersion = VK_API_VERSION_1_0;

	//uint32_t extensionCount = 0;
	//SDL_Vulkan_GetInstanceExtensions(Window, &extensionCount, nullptr);

	//std::vector<const char*> extensions(extensionCount);
	//SDL_Vulkan_GetInstanceExtensions(Window, &extensionCount, extensions.data());

	//for (const char* extension : AdditionalExtensions)
	//{
	//	extensions.push_back(extension);
	//}

	//VkInstanceCreateInfo createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	//createInfo.pApplicationInfo = &applicationInfo;
	//createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	//createInfo.ppEnabledExtensionNames = extensions.data();
	//createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	//createInfo.ppEnabledLayerNames = ValidationLayers.data();

	//VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
	//if (result != VK_SUCCESS)
	//{
	//	std::cout << "FAIL\n";
	//	return;
	//}
}

void Engine::CreateSurface()
{
	//SDL_Vulkan_CreateSurface(Window, Instance, &Surface);
}

void Engine::SelectPhysicalDevice()
{
	//uint32_t deviceCount = 0;
	//vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

	//std::vector<VkPhysicalDevice> devices(deviceCount);
	//vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

	//std::cout << deviceCount << " compatible physical device(s)\n";
	//for (VkPhysicalDevice device : devices)
	//{
	//	VkPhysicalDeviceProperties deviceProperties;
	//	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//	std::cout << deviceProperties.deviceName << "\n";
	//}

	//PhysicalDevice = devices[0];
	//std::cout << "First physical device selected\n";
}

void Engine::CreateDevice()
{
	//// Queue Families
	//uint32_t queueFamilyCount = 0;
	//vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);

	//std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	//vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	//bool foundFamily = false;
	//for (uint32_t i = 0; i < queueFamilies.size(); i++)
	//{
	//	const auto& family = queueFamilies[i];

	//	// TODO: advanced swapchain present queue selection logic
	//	VkBool32 supportsPresent = false;
	//	vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &supportsPresent);

	//	if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT && supportsPresent)
	//	{
	//		GraphicsFamily = i;
	//		foundFamily = true;

	//		break;
	//	}
	//}
	//if (!foundFamily)
	//{
	//	std::cout << "no family found\n";
	//	return; // err
	//}

	//const float queuePriority = 1.0f;

	//// TODO: for multiple queues, std::vector
	//VkDeviceQueueCreateInfo queueCreateInfo = {};
	//queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//queueCreateInfo.queueFamilyIndex = GraphicsFamily;
	//queueCreateInfo.queueCount = 1;
	//queueCreateInfo.pQueuePriorities = &queuePriority;

	//VkPhysicalDeviceFeatures deviceFeatures = {};

	//// Device
	//VkDeviceCreateInfo createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	//createInfo.pQueueCreateInfos = &queueCreateInfo;
	//createInfo.queueCreateInfoCount = 1;
	//createInfo.pEnabledFeatures = &deviceFeatures;
	//createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	//createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	//VkResult result = vkCreateDevice(PhysicalDevice, &createInfo, nullptr, &Device);
	//if (result != VK_SUCCESS)
	//{
	//	// err
	//	std::cout << "vk create device failed\n";
	//}

	//vkGetDeviceQueue(Device, GraphicsFamily, 0, &GraphicsQueue);

}

void Engine::CreateMemoryAllocator()
{
	//VmaAllocatorCreateInfo createInfo = {};
	//createInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	//createInfo.physicalDevice = PhysicalDevice;
	//createInfo.device = Device;
	//createInfo.instance = Instance;

	//vmaCreateAllocator(&createInfo, &Allocator);
}

void Engine::CreateSwapchain()
{
	//VkSurfaceCapabilitiesKHR capabilities;
	//vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &capabilities);

	//uint32_t formatCount = 0;
	//vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, nullptr);

	//std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	//vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &formatCount, availableFormats.data());

	//VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
	//VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	//VkExtent2D extent = { winWidth, winHeight };

	//uint32_t imageCount = capabilities.minImageCount + 1; // not mindful of max image count !

	//VkSwapchainCreateInfoKHR createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	//createInfo.surface = Surface;
	//createInfo.minImageCount = imageCount;
	//createInfo.imageFormat = surfaceFormat.format;
	//createInfo.imageColorSpace = surfaceFormat.colorSpace;
	//createInfo.imageExtent = extent;
	//createInfo.imageArrayLayers = 1;
	//createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//createInfo.preTransform = capabilities.currentTransform;
	//createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//createInfo.presentMode = presentMode;
	//createInfo.clipped = VK_TRUE;
	//createInfo.oldSwapchain = VK_NULL_HANDLE;

	//VkResult result = vkCreateSwapchainKHR(Device, &createInfo, nullptr, &Swapchain);
	//if (result != VK_SUCCESS)
	//{
	//	std::cout << "Swapchain failed to create\n";
	//	return;
	//}

	//vkGetSwapchainImagesKHR(Device, Swapchain, &imageCount, nullptr);

	//SwapChainImages.resize(imageCount);
	//vkGetSwapchainImagesKHR(Device, Swapchain, &imageCount, SwapChainImages.data());

	//SwapChainImageFormat = surfaceFormat.format;
	//SwapChainExtent = extent;
}

void Engine::CreateImageViews()
{
	//SwapChainImageViews.resize(SwapChainImages.size());

	//for (size_t i = 0; i < SwapChainImages.size(); i++)
	//{
	//	VkImageViewCreateInfo createInfo = {};
	//	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	//	createInfo.image = SwapChainImages[i];
	//	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	//	createInfo.format = SwapChainImageFormat;

	//	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	//	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	//	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	//	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	//	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	createInfo.subresourceRange.baseMipLevel = 0;
	//	createInfo.subresourceRange.levelCount = 1;
	//	createInfo.subresourceRange.baseArrayLayer = 0;
	//	createInfo.subresourceRange.layerCount = 1;

	//	VkResult result = vkCreateImageView(Device, &createInfo, nullptr, &SwapChainImageViews[i]);
	//	if (result != VK_SUCCESS)
	//	{
	//		std::cout << "image view fail!\n";
	//		return;
	//	}
	//}
}

void Engine::CreateDescriptorSetLayout()
{
	//VkDescriptorSetLayoutBinding layoutBinding = {};
	//layoutBinding.binding = 0;
	//layoutBinding.descriptorCount = 1;
	//layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	//VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	//layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//layoutInfo.bindingCount = 1;
	//layoutInfo.pBindings = &layoutBinding;

	//VkResult result = vkCreateDescriptorSetLayout(NewDevice->Device, &layoutInfo, nullptr, &descriptorSetLayout);
	//CRITICAL_ASSERT(result == VK_SUCCESS, "bind create fail");
}

void Engine::CreateGraphicsPipeline()
{
	//
	auto vertexShaderCode = File::ReadAllBytes("data/vertex.spv");
	auto fragmentShaderCode = File::ReadAllBytes("data/fragment.spv");

	VkShaderModule vertexShaderModule = CreateShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertexShaderStage = {};
	vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStage.module = vertexShaderModule;
	vertexShaderStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStage = {};
	fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStage.module = fragmentShaderModule;
	fragmentShaderStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStage, fragmentShaderStage };

	//
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInput = {};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &bindingDescription;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

	//
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(winWidth);
	viewport.height = static_cast<float>(winHeight);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { winWidth, winHeight };

	//
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	//
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//
	VkPipelineDepthStencilStateCreateInfo depthStencil = {}; // using nullptr til we need depthtesting

	//
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE; // no blending !

	//
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; // disable!
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	// we only care about resizing viewport without having to recreate pipelines
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {}; // UNUSED !!!!!!!!!!!!!!!!
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 1;
	dynamicState.pDynamicStates = dynamicStates;

	//
	VkPipelineLayoutCreateInfo pipelineLayout = {};
	pipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	VkResult result = vkCreatePipelineLayout(NewDevice->Device, &pipelineLayout, nullptr, &PipelineLayout);
	if (result != VK_SUCCESS)
	{
		std::cout << "pipeline layout\n";
		return;
	}

	// Create
	VkGraphicsPipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStages;
	createInfo.pVertexInputState = &vertexInput;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pViewportState = &viewportState;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisampling;
	createInfo.pDepthStencilState = nullptr; // !!!!!!!!
	createInfo.pColorBlendState = &colorBlending;
	createInfo.pDynamicState = nullptr;
	createInfo.layout = PipelineLayout;
	createInfo.renderPass = NewDevice->Swapchain.RenderPass;
	createInfo.subpass = 0;

	result = vkCreateGraphicsPipelines(NewDevice->Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &GraphicsPipeline);
	if (result != VK_SUCCESS)
	{
		std::cout << "ouch graphics pipeline\n";
		return;
	}

	//
	vkDestroyShaderModule(NewDevice->Device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(NewDevice->Device, vertexShaderModule, nullptr);
}

void Engine::CreateRenderPass()
{
	//VkAttachmentDescription colorAttachment = {};
	//colorAttachment.format = NewSwapChain.SwapChainImageFormat; //SwapChainImageFormat;
	//colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//VkAttachmentReference colorAttachmentReference = {};
	//colorAttachmentReference.attachment = 0;
	//colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//VkSubpassDescription subpass = {};
	//subpass.colorAttachmentCount = 1;
	//subpass.pColorAttachments = &colorAttachmentReference;

	//VkRenderPassCreateInfo createInfo = {};
	//createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//createInfo.attachmentCount = 1;
	//createInfo.pAttachments = &colorAttachment;
	//createInfo.subpassCount = 1;
	//createInfo.pSubpasses = &subpass;

	//VkResult result = vkCreateRenderPass(Device, &createInfo, nullptr, &RenderPass);
	//if (result != VK_SUCCESS)
	//{
	//	std::cout << "render pass fail\n";
	//	return;
	//}

}

void Engine::CreateFramebuffers()
{
	//SwapChainFramebuffers.resize(NewSwapChain.SwapChainImageViews.size());

	//for (size_t i = 0; i < NewSwapChain.SwapChainImageViews.size(); i++)
	//{
	//	VkImageView attachments[] = { NewSwapChain.SwapChainImageViews[i] };

	//	VkFramebufferCreateInfo createInfo = {};
	//	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//	createInfo.renderPass = RenderPass;
	//	createInfo.attachmentCount = 1;
	//	createInfo.pAttachments = attachments;
	//	createInfo.width = winWidth;
	//	createInfo.height = winHeight;
	//	createInfo.layers = 1;

	//	VkResult result = vkCreateFramebuffer(Device, &createInfo, nullptr, &SwapChainFramebuffers[i]);
	//	if (result != VK_SUCCESS)
	//	{
	//		std::cout << "framebuffer " << i << " failed\n";
	//		return;
	//	}
	//}
}

void Engine::CreateCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = NewDevice->GraphicsFamily;

	VkResult result = vkCreateCommandPool(NewDevice->Device, &createInfo, nullptr, &CommandPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "command pool\n";
		return;
	}
}

void Engine::CreateCommandBuffers()
{
	CommandBuffers.resize(NewDevice->Swapchain.SwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocation = {};
	allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation.commandPool = CommandPool;
	allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(NewDevice->Device, &allocation, CommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		std::cout << "Command buffers fail\n";
		return;
	}

	for (size_t i = 0; i < CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		result = vkBeginCommandBuffer(CommandBuffers[i], &bufferInfo);
		if (result != VK_SUCCESS)
		{
			std::cout << "recording failed\n";
			return;
		}

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f }} };

		VkRenderPassBeginInfo passInfo = {};
		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		passInfo.renderPass = NewDevice->Swapchain.RenderPass;
		passInfo.framebuffer = NewDevice->Swapchain.SwapChainFramebuffers[i];
		passInfo.renderArea.offset = { 0, 0 };
		passInfo.renderArea.extent = { winWidth, winHeight };
		passInfo.clearValueCount = 1;
		passInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(CommandBuffers[i], &passInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

		VkBuffer vertexBuffers[] = { VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(CommandBuffers[i], IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		//vkCmdDraw(CommandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		vkCmdDrawIndexed(CommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(CommandBuffers[i]);

		result = vkEndCommandBuffer(CommandBuffers[i]);
		if (result != VK_SUCCESS)
		{
			std::cout << "recording failed 2\n";
			return;
		}
	}
}

void Engine::CreateVertexBuffer()
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationInfo = {};
	allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	VkResult result = vmaCreateBuffer(Allocator, &bufferInfo, &allocationInfo, &VertexBuffer, &VertexBufferAllocation, nullptr);
	if (result != VK_SUCCESS)
	{
		std::cout << "alloc failed\n";
		return;
	}

	void* data;
	vmaMapMemory(Allocator, VertexBufferAllocation, &data);
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vmaUnmapMemory(Allocator, VertexBufferAllocation);

	//  vmaFlushAllocation() // might be necessary eventually someday
}

void Engine::CreateIndexBuffer()
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(indices[0]) * indices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationInfo = {};
	allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	vmaCreateBuffer(Allocator, &bufferInfo, &allocationInfo, &IndexBuffer, &IndexBufferAllocation, nullptr);

	void* data;
	vmaMapMemory(Allocator, IndexBufferAllocation, &data);
	memcpy(data, indices.data(), (size_t)bufferInfo.size);
	vmaUnmapMemory(Allocator, IndexBufferAllocation);
}

void Engine::CreateSemaphores()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_AHEAD);
	renderFinishedSemaphores.resize(MAX_FRAMES_AHEAD);
	inFlightFences.resize(MAX_FRAMES_AHEAD);
	imagesInFlight.resize(NewDevice->Swapchain.SwapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // first frame 

	for (uint32_t i = 0; i < MAX_FRAMES_AHEAD; i++)
	{
		// lol ignore result
		vkCreateSemaphore(NewDevice->Device, &createInfo, nullptr, &imageAvailableSemaphores[i]);
		vkCreateSemaphore(NewDevice->Device, &createInfo, nullptr, &renderFinishedSemaphores[i]);
		vkCreateFence(NewDevice->Device, &fenceInfo, nullptr, &inFlightFences[i]);
	}
}


void Engine::RecreateSwapchain()
{
	std::cout << "Recreating swapchain\n";
	vkDeviceWaitIdle(NewDevice->Device);

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandBuffers();
}

VkShaderModule Engine::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(NewDevice->Device, &createInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		std::cout << "oops shader\n";
		return nullptr;
	}
	return shaderModule;
}