#include "pipeline.hpp"
#include <vulkan/vulkan_core.h>

VulkanPipeline::VulkanPipeline(VulkanInstance& instance) : m_instance(instance) {
	createRenderPass();
	createGraphicsPipeline();
}

void VulkanPipeline::createRenderPass() {
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = m_instance.getSwapChainFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear image before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save rendered image to display
	colorAttachment.stencilLoadOp =
		VK_ATTACHMENT_LOAD_OP_DONT_CARE; // We don't use the stencil buffer
	colorAttachment.stencilStoreOp =
		VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't use the stencil buffer
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout =
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Ultimately, we want the color buffer to display to the
	                                     // screen

	// create reference to attached image
	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0; // TODO: this should be abstracted
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Define subpass to do rendering
	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // this array is index by frag shader outputs

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(m_instance.getLogicalDevice(), &renderPassInfo, nullptr,
	                       &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanPipeline::createGraphicsPipeline() {
	auto vertShaderCode = readFile("res/shaderc/triangle.vert.spv");
	auto fragShaderCode = readFile("res/shaderc/triangle.frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// Bind each shader to appropriate pipeline stage
	VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	// Specify this pipelines dynamic state (i.e. vars that can be changed w/o recreation)
	/* std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
	                                             VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data(); */

	// Specify inputs (uniforms) expected by vertex shader
	// TODO: abstract this logic to a shader class
	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // spacing between data
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // types of attributes passed

	// Create state for fixed function pipeline stages
	PipelineConfigInfo pi = defaultPipelineConfigInfo();

	// Create pipeline layout (TODO: not sure what this is for)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;            // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(m_instance.getLogicalDevice(), &pipelineLayoutInfo, nullptr,
	                           &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Create graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pi.inputAssemblyInfo;
	pipelineInfo.pViewportState = &pi.viewportInfo;
	pipelineInfo.pRasterizationState = &pi.rasterizationInfo;
	pipelineInfo.pMultisampleState = &pi.multisampleInfo;
	pipelineInfo.pDepthStencilState = &pi.depthStencilInfo;
	pipelineInfo.pColorBlendState = &pi.colorBlendInfo;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0; // index of the subpass in the render pass to use this pipeline
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // We aren't inheriting from another pipeline
	pipelineInfo.basePipelineIndex = -1;              // We aren't inheriting from another pipeline

	if (vkCreateGraphicsPipelines(m_instance.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
	                              nullptr, &m_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// shader modules linked to GPU, can destroy local copy
	vkDestroyShaderModule(m_instance.getLogicalDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(m_instance.getLogicalDevice(), vertShaderModule, nullptr);
}

VulkanPipeline::~VulkanPipeline() {
	vkDestroyPipeline(m_instance.getLogicalDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_instance.getLogicalDevice(), m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_instance.getLogicalDevice(), m_renderPass, nullptr);
}

std::vector<char> VulkanPipeline::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filename);
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	// read entire file contents into buffer
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

VkShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_instance.getLogicalDevice(), &createInfo, nullptr, &shaderModule) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

PipelineConfigInfo VulkanPipeline::defaultPipelineConfigInfo() {
	PipelineConfigInfo configInfo {};

	// Specify the geometry primitive to use
	configInfo.inputAssemblyInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	// Screen coordinate locations to render to
	configInfo.viewport.x = 0.0f;
	configInfo.viewport.y = 0.0f;
	configInfo.viewport.width = m_instance.getSwapChainExtent().width;
	configInfo.viewport.height = m_instance.getSwapChainExtent().height;
	configInfo.viewport.minDepth = 0.0f;
	configInfo.viewport.maxDepth = 1.0f;

	// Pixels outside of this screen coordinate region are simply discarded
	// Does nothing if this region is larger than the viewport
	configInfo.scissor.offset = {0, 0};
	configInfo.scissor.extent = {m_instance.getSwapChainExtent().width,
	                             m_instance.getSwapChainExtent().height};

	// Known issue: this creates a self-referencing structure. Fixed in tutorial 05
	configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.viewportInfo.viewportCount = 1;
	configInfo.viewportInfo.pViewports = &configInfo.viewport;
	configInfo.viewportInfo.scissorCount = 1;
	configInfo.viewportInfo.pScissors = &configInfo.scissor;

	// Controls which pixels get mapped to what geometry
	configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.rasterizationInfo.depthClampEnable =
		VK_FALSE; // discard objects outside of depth region
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE; // do not disable rasterizer
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth = 1.0f;
	configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
	configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
	configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

	// Use information from >1 geometry to color pixels on borders (disabled for now)
	configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	configInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
	configInfo.multisampleInfo.pSampleMask = nullptr;            // Optional
	configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

	// Controls how to blend colors (local to framebuffer). Works like:
	/* if (blendEnable) {
	    finalColor.rgb =
	        (srcColorBlendFactor * newColor.rgb)<colorBlendOp>(dstColorBlendFactor * oldColor.rgb);
	    finalColor.a =
	        (srcAlphaBlendFactor * newColor.a)<alphaBlendOp>(dstAlphaBlendFactor * oldColor.a);
	} else {
	    finalColor = newColor;
	}

	finalColor = finalColor & colorWriteMask; */
	configInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
	configInfo.colorBlendAttachment.dstColorBlendFactor =
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;                                    // Optional
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

	// Alternative (global) way to describe color blending (disabled for now)
	configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
	configInfo.colorBlendInfo.attachmentCount = 1;
	configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

	configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
	configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
	configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.front = {}; // Optional
	configInfo.depthStencilInfo.back = {};  // Optional

	return configInfo;
}
