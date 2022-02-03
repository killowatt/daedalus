#include "VulkanSwapChain.h"

#include "VulkanDevice.h"
#include "Common.h"

VulkanSwapchain::VulkanSwapchain()
{
}

void VulkanSwapchain::Create(uint32_t width, uint32_t height)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device->PhysicalDevice, Surface, &capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device->PhysicalDevice, Surface, &formatCount, nullptr);

	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device->PhysicalDevice, Surface, &formatCount, availableFormats.data());

	// Format
	VkSurfaceFormatKHR surfaceFormat = availableFormats[0]; // Just pick the first available format

	// Present Mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // No vsync

	// Extent
	VkExtent2D extent = {};
	if (capabilities.currentExtent.width == UINT32_MAX ||
		capabilities.currentExtent.height == UINT32_MAX)
	{
		extent.width = width;
		extent.height = height;
	}
	else
	{
		extent = capabilities.currentExtent;
	}

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = Surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.preTransform = capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(Device->Device, &swapchainInfo, nullptr, &Swapchain);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Swapchain creation failed");

	SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Device->Device, Swapchain, &imageCount, SwapChainImages.data());

	SwapChainImageFormat = surfaceFormat.format;
	SwapChainExtent = extent;

	// Image Views
	SwapChainImageViews.resize(imageCount);
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

		VkResult result = vkCreateImageView(Device->Device, &createInfo, nullptr, &SwapChainImageViews[i]);
		CRITICAL_ASSERT(result == VK_SUCCESS, "Swapchain creation failed");
	}

	// Render Pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = SwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAttachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;

	result = vkCreateRenderPass(Device->Device, &createInfo, nullptr, &RenderPass);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Swapchain creation failed");

	// Framebuffers
	SwapChainFramebuffers.resize(SwapChainImageViews.size());

	for (size_t i = 0; i < SwapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = { SwapChainImageViews[i] };

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = RenderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = attachments;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.layers = 1;

		result = vkCreateFramebuffer(Device->Device, &createInfo, nullptr, &SwapChainFramebuffers[i]);
		CRITICAL_ASSERT(result == VK_SUCCESS, "Swapchain!!!");
	}


	// Endlog
	printf("# of images : %zd\n", SwapChainImages.size());
}

int VulkanSwapchain::NextImage(VkSemaphore semaphore)
{
	VkResult result = vkAcquireNextImageKHR(Device->Device, Swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &CurrentImage);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		CRITICAL_ERROR("WOOPS");
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		CRITICAL_ERROR("swapchain bad");
	}

	return CurrentImage;
}

void VulkanSwapchain::Present(VkSemaphore waitSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &Swapchain;
	presentInfo.pImageIndices = &CurrentImage;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;

	VkResult result = vkQueuePresentKHR(PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR)
	{
		CRITICAL_ERROR("swapchain suboptimial/out of date");
	}

	//currentFrame = (currentFrame + 1) % maxFramesInFlight;
}