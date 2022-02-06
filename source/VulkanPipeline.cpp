#include "VulkanPipeline.h"

#include "Common.h"
#include "VulkanDevice.h"

VulkanPipeline* VulkanPipeline::Create(VulkanDevice* device, const VulkanShader* const shader, std::vector<VertexAttribute> attributes, uint32_t vertexSize)
{
	VulkanPipeline* pipeline = new VulkanPipeline();

	VkShaderModule vertexShaderModule = CreateShader(device->Device, shader->VertexBytes);
	VkShaderModule fragmentShaderModule = CreateShader(device->Device, shader->FragmentBytes);

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

	// vertex binding
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = vertexSize;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// vertex attribs
	std::vector<VkVertexInputAttributeDescription> attribs(attributes.size());
	for (size_t i = 0; i < attributes.size(); i++)
	{
		VertexAttribute& attribute = attributes[i];

		VkFormat format;
		switch (attribute.Type)
		{
		case AttributeType::Float2: format = VK_FORMAT_R32G32_SFLOAT; break;
		case AttributeType::Float3: format = VK_FORMAT_R32G32B32_SFLOAT; break;
		}

		attribs[i].binding = 0;
		attribs[i].location = static_cast<uint32_t>(i);
		attribs[i].format = format;
		attribs[i].offset = attribute.Offset;
	}

	VkPipelineVertexInputStateCreateInfo vertexInput = {};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &bindingDescription;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size());
	vertexInput.pVertexAttributeDescriptions = attribs.data();

	//
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(device->Swapchain.Extent.width); // TODO:
	viewport.height = static_cast<float>(device->Swapchain.Extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	//
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = device->Swapchain.Extent; // TODO:

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

	VkResult result = vkCreatePipelineLayout(device->Device, &pipelineLayout, nullptr, &pipeline->PipelineLayout);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Pipeline layout creation failed");

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
	createInfo.layout = pipeline->PipelineLayout;
	createInfo.renderPass = device->Swapchain.RenderPass;
	createInfo.subpass = 0;

	result = vkCreateGraphicsPipelines(device->Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline->Pipeline);
	CRITICAL_ASSERT(result == VK_SUCCESS, "Graphics pipeline creation failed");

	//
	vkDestroyShaderModule(device->Device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(device->Device, vertexShaderModule, nullptr);

	return pipeline;
}

VkShaderModule VulkanPipeline::CreateShader(VkDevice device, std::vector<uint8_t> bytes)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytes.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);


	return shaderModule;
}

//void VulkanPipeline::SetShader(const VulkanShader* const shader)
//{
//
//}