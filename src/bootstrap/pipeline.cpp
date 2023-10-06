#include "pipeline.hpp"

#include "instance.hpp"
#include "util/constants.hpp"
#include "util/profiler.hpp"

#include <cstring>
#include <vulkan/vulkan_core.h>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VulkanPipeline::VulkanPipeline(VulkanDevice& device, const VulkanSwapChain& swapChain,
                               VkImageView imageView)
	: m_device(device), m_swapChain(swapChain), m_imageView(imageView) {
	// FIXME: set up reasonable default vertex attributes, descriptors
}

VulkanPipeline::~VulkanPipeline() {
	vkDestroySampler(m_device.getLogicalDevice(), m_textureSampler, nullptr);

	for (uint32_t i = 0; i < m_uniformBuffers.size(); i++) {
		for (uint32_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
			// Destroy uniform buffers
			vkDestroyBuffer(m_device.getLogicalDevice(), m_uniformBuffers[i][j], nullptr);
			vkFreeMemory(m_device.getLogicalDevice(), m_uniformBuffersMemory[i][j], nullptr);
		}
	}

	vkDestroyDescriptorPool(m_device.getLogicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_device.getLogicalDevice(), m_descriptorSetLayout, nullptr);
	vkDestroyPipeline(m_device.getLogicalDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device.getLogicalDevice(), m_pipelineLayout, nullptr);
}

void VulkanPipeline::create() {
	createDescriptorSetLayout();
	createTextureSampler();
	createDescriptorPool();
	createDescriptorSets(m_imageView);
	createGraphicsPipeline(m_vertexAttrBindings, m_vertexAttr, m_swapChain.getRenderPass());
}

void VulkanPipeline::setVertexArray(const VertexArray& vertexArray) {
	m_vertexAttrBindings = vertexArray.getBindingDescription();
	m_vertexAttr = vertexArray.getAttributeDescriptions();
}

uint32_t VulkanPipeline::pushUniform(VkShaderStageFlags stage, uint32_t size) {
	uint32_t uniformID = m_bindings.size();

	// TODO: What does this do?
	VkDescriptorSetLayoutBinding binding {};
	binding.binding = uniformID; // index of binding, order specified in shader code
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.descriptorCount = 1;
	binding.stageFlags = stage;
	binding.pImmutableSamplers = nullptr; // not doing image sampling yet

	m_bindings.push_back(binding);
	m_uniformSizes.push_back(size);

	// Allocate memory on GPU to store uniform
	// TODO: I should be able to do this with one giant buffer for all uniforms
	Frames<VkBuffer> uniformBuffers;
	Frames<VkDeviceMemory> uniformBufferDeviceMemory;
	Frames<void*> uniformBuffersMapped;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_device.createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                      uniformBuffers[i], uniformBufferDeviceMemory[i]);

		vkMapMemory(m_device.getLogicalDevice(), uniformBufferDeviceMemory[i], 0, size, 0,
		            &uniformBuffersMapped[i]);
	}

	m_uniformBuffers.push_back(uniformBuffers);
	m_uniformBuffersMemory.push_back(uniformBufferDeviceMemory);
	m_uniformBuffersMapped.push_back(uniformBuffersMapped);

	return uniformID;
}

void VulkanPipeline::bind(VkCommandBuffer commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void VulkanPipeline::bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
	                        &m_descriptorSets[currentFrame], 0, nullptr);
}

void VulkanPipeline::writeUniform(uint32_t uniformID, void* data, uint32_t currentFrame) {
	memcpy(m_uniformBuffersMapped[uniformID][currentFrame], data, m_uniformSizes[uniformID]);
}

void VulkanPipeline::createGraphicsPipeline(VkVertexInputBindingDescription bindingDesc,
                                            std::vector<VkVertexInputAttributeDescription> attrDesc,
                                            VkRenderPass renderPass) {
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

	// Specify bind point to incorporate vertex buffer into pipeline
	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDesc; // spacing between data
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions =
		attrDesc.data(); // type, size, and order of attributes passed

	// Create state for fixed function pipeline stages
	PipelineConfigInfo pi = defaultPipelineConfigInfo();

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;                   // Optional
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;           // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr;        // Optional

	if (vkCreatePipelineLayout(m_device.getLogicalDevice(), &pipelineLayoutInfo, nullptr,
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
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0; // index of the subpass in the render pass to use this pipeline
	                          // FIXME: this is now arbitrary
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // We aren't inheriting from another pipeline
	pipelineInfo.basePipelineIndex = -1;              // We aren't inheriting from another pipeline

	if (vkCreateGraphicsPipelines(m_device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
	                              nullptr, &m_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// shader modules linked to GPU, can destroy local copy
	vkDestroyShaderModule(m_device.getLogicalDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(m_device.getLogicalDevice(), vertShaderModule, nullptr);
}

void VulkanPipeline::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = m_device.getMaxAnistropy();
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(m_device.getLogicalDevice(), &samplerInfo, nullptr, &m_textureSampler) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
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
	if (vkCreateShaderModule(m_device.getLogicalDevice(), &createInfo, nullptr, &shaderModule) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void VulkanPipeline::createDescriptorSetLayout() {
	// We have list of uniform bindings to be used for this pipeline

	// Add image sampler binding
	VkDescriptorSetLayoutBinding samplerLayoutBinding {};
	samplerLayoutBinding.binding = m_bindings.size();
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	m_bindings.push_back(samplerLayoutBinding);

	// Group bindings into a descriptor set
	VkDescriptorSetLayoutCreateInfo layoutInfo {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
	layoutInfo.pBindings = m_bindings.data();

	if (vkCreateDescriptorSetLayout(m_device.getLogicalDevice(), &layoutInfo, nullptr,
	                                &m_descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descripto  set layout!");
	}
}

void VulkanPipeline::createDescriptorPool() {
	// Create descriptor for for each uniform and frame in flight
	for (uint32_t i = 0; i < m_uniformBuffers.size(); i++) {
		VkDescriptorPoolSize uniformBufferPoolSize {};
		uniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		m_poolSizes.push_back(uniformBufferPoolSize);
	}

	// Create descriptor for image sampler for each frame in flight
	VkDescriptorPoolSize imageSamplerPoolSize;
	imageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageSamplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	m_poolSizes.push_back(imageSamplerPoolSize);

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount =
		static_cast<uint32_t>(m_poolSizes.size()); // number of different types of descriptors
	poolInfo.pPoolSizes = m_poolSizes.data();      // limits count of a certain type of descriptor
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); // max total descriptors in pool

	if (vkCreateDescriptorPool(m_device.getLogicalDevice(), &poolInfo, nullptr,
	                           &m_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanPipeline::createDescriptorSets(VkImageView imageView) {
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);

	// Allocate descriptor sets
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(m_device.getLogicalDevice(), &allocInfo,
	                             m_descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	// Populate descriptor sets (each one takes one MVP uniform and one image sampler)
	for (uint32_t frameIdx = 0; frameIdx < MAX_FRAMES_IN_FLIGHT; frameIdx++) {
		// Add descriptor for each uniform buffer
		std::vector<VkDescriptorBufferInfo> bufferInfos(m_uniformBuffers.size());

		for (uint32_t uniformIdx = 0; uniformIdx < m_uniformBuffers.size(); uniformIdx++) {
			VkDescriptorBufferInfo bufferInfo {};
			bufferInfo.buffer = m_uniformBuffers[uniformIdx][frameIdx];
			bufferInfo.offset = 0;
			bufferInfo.range = m_uniformSizes[uniformIdx];

			bufferInfos[uniformIdx] = bufferInfo;
		}

		// Add descriptor for texture
		VkDescriptorImageInfo imageInfo {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = m_textureSampler;

		// Describe how to write to the descriptor set
		std::vector<VkWriteDescriptorSet> descriptorWrites(m_uniformBuffers.size() + 1);

		for (uint32_t uniformIdx = 0; uniformIdx < m_uniformBuffers.size(); uniformIdx++) {
			descriptorWrites[uniformIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[uniformIdx].dstSet = m_descriptorSets[frameIdx];
			descriptorWrites[uniformIdx].dstBinding = uniformIdx;
			descriptorWrites[uniformIdx].dstArrayElement = 0;
			descriptorWrites[uniformIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[uniformIdx].descriptorCount = 1;
			descriptorWrites[uniformIdx].pBufferInfo = &bufferInfos[uniformIdx];
		}

		uint32_t samplerIdx = descriptorWrites.size() - 1;

		descriptorWrites[samplerIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[samplerIdx].dstSet = m_descriptorSets[frameIdx];
		descriptorWrites[samplerIdx].dstBinding = samplerIdx;
		descriptorWrites[samplerIdx].dstArrayElement = 0;
		descriptorWrites[samplerIdx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[samplerIdx].descriptorCount = 1;
		descriptorWrites[samplerIdx].pImageInfo = &imageInfo;

		// Write to descriptor set
		vkUpdateDescriptorSets(m_device.getLogicalDevice(),
		                       static_cast<uint32_t>(descriptorWrites.size()),
		                       descriptorWrites.data(), 0, nullptr);
	}
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
	configInfo.viewport.width = m_swapChain.getExtent().width;
	configInfo.viewport.height = m_swapChain.getExtent().height;
	configInfo.viewport.minDepth = 0.0f;
	configInfo.viewport.maxDepth = 1.0f;

	// Pixels outside of this screen coordinate region are simply discarded
	// Does nothing if this region is larger than the viewport
	configInfo.scissor.offset = {0, 0};
	configInfo.scissor.extent = {m_swapChain.getExtent().width, m_swapChain.getExtent().height};

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
	configInfo.rasterizationInfo.frontFace =
		VK_FRONT_FACE_COUNTER_CLOCKWISE; // needed because we flip to compensate for glm assuming
	                                     // OpenGL
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
