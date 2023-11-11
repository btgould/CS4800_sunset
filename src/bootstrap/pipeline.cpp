#include "pipeline.hpp"

#include "instance.hpp"
#include "util/constants.hpp"
#include "util/memory.hpp"
#include "util/profiler.hpp"

#include <cstring>
#include <vulkan/vulkan_core.h>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VulkanPipeline::VulkanPipeline(VulkanDevice& device, const VulkanSwapChain& swapChain)
	: m_device(device), m_swapChain(swapChain) {
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
	vkDestroyDescriptorSetLayout(m_device.getLogicalDevice(), m_uniformLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device.getLogicalDevice(), m_textureLayout, nullptr);
	vkDestroyPipeline(m_device.getLogicalDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device.getLogicalDevice(), m_pipelineLayout, nullptr);
}

void VulkanPipeline::create() {
	createDescriptorSetLayout();
	createTextureSampler();
	createDescriptorPool();
	createDescriptorSets();
	createGraphicsPipeline(m_vertexAttrBindings, m_vertexAttr, m_swapChain.getRenderPass());
}

void VulkanPipeline::setVertexArray(const VertexArray& vertexArray) {
	m_vertexAttrBindings = vertexArray.getBindingDescription();
	m_vertexAttr = vertexArray.getAttributeDescriptions();
}

uint32_t VulkanPipeline::pushUniform(VkShaderStageFlags stage, uint32_t size) {
	uint32_t uniformID = m_uniformBindings.size();

	VkDescriptorSetLayoutBinding binding {};
	binding.binding = uniformID; // index of binding, order specified in shader code
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.descriptorCount = 1;
	binding.stageFlags = stage;
	binding.pImmutableSamplers = nullptr; // not doing image sampling yet

	m_uniformBindings.push_back(binding);
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

void VulkanPipeline::pushTexture(const Ref<Texture> tex) {
	VkDescriptorSetLayoutBinding samplerLayoutBinding {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	m_textureBindings.push_back(samplerLayoutBinding);
	m_textureIdx[tex] = m_textures.size();
	m_textures.push_back(tex);
}

void VulkanPipeline::bind(VkCommandBuffer commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void VulkanPipeline::bindTexture(Ref<Texture> tex) {
	m_activeTex = tex;
}

void VulkanPipeline::bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
	m_activeDescriptorSets[0] = m_uniformDescriptorSets[currentFrame];
	m_activeDescriptorSets[1] =
		m_textureDescriptorSets[m_textureIdx[m_activeTex]]
							   [currentFrame]; // TODO: should maybe consider error handling here
	// Map insertion will happen automatically , but isn't guaranteed to line up w/ descriptor set
	// order / len
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 2,
	                        m_activeDescriptorSets.data(), 0, nullptr);
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
	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {m_uniformLayout, m_textureLayout};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

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
	// Bindings for uniforms stored in m_uniformBindings
	// Bindings for textures stored in m_textureBindings

	// Create layout for all uniform bindings in one descriptor set
	VkDescriptorSetLayoutCreateInfo uniformLayoutInfo {};
	uniformLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	uniformLayoutInfo.bindingCount = static_cast<uint32_t>(m_uniformBindings.size());
	uniformLayoutInfo.pBindings = m_uniformBindings.data();

	if (vkCreateDescriptorSetLayout(m_device.getLogicalDevice(), &uniformLayoutInfo, nullptr,
	                                &m_uniformLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	// Create layout for descriptor set with one texture
	VkDescriptorSetLayoutBinding samplerLayoutBinding {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo textureLayoutInfo {};
	textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutInfo.bindingCount = 1;
	textureLayoutInfo.pBindings = &samplerLayoutBinding;

	if (vkCreateDescriptorSetLayout(m_device.getLogicalDevice(), &textureLayoutInfo, nullptr,
	                                &m_textureLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void VulkanPipeline::createDescriptorPool() {
	// Create descriptor for for each uniform and frame in flight
	for (const auto& uniform : m_uniformBuffers) {
		VkDescriptorPoolSize uniformBufferPoolSize {};
		uniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		m_poolSizes.push_back(uniformBufferPoolSize);
	}

	// Create descriptor for image sampler for each frame in flight
	for (const auto& texture : m_textures) {
		VkDescriptorPoolSize imageSamplerPoolSize;
		imageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageSamplerPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		m_poolSizes.push_back(imageSamplerPoolSize);
	}

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
	poolInfo.pPoolSizes = m_poolSizes.data(); // describes number and type of different descriptors
	poolInfo.maxSets =
		(m_textures.size() + 1) *
		static_cast<uint32_t>(
			MAX_FRAMES_IN_FLIGHT); // max number of descriptor sets allocated at a time

	if (vkCreateDescriptorPool(m_device.getLogicalDevice(), &poolInfo, nullptr,
	                           &m_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanPipeline::createDescriptorSets() {
	// Allocate uniform descriptor sets (1 for each frame)
	std::vector<VkDescriptorSetLayout> uniformLayouts(MAX_FRAMES_IN_FLIGHT, m_uniformLayout);

	VkDescriptorSetAllocateInfo uniformAllocInfo {};
	uniformAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	uniformAllocInfo.descriptorPool = m_descriptorPool;
	uniformAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	uniformAllocInfo.pSetLayouts = uniformLayouts.data();

	if (vkAllocateDescriptorSets(m_device.getLogicalDevice(), &uniformAllocInfo,
	                             m_uniformDescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate uniform descriptor sets!");
	}

	// Allocate texture descriptor sets (1 for each texture and frame)
	std::vector<VkDescriptorSetLayout> textureLayouts(MAX_FRAMES_IN_FLIGHT, m_textureLayout);

	m_textureDescriptorSets.resize(m_textures.size());
	for (uint32_t i = 0; i < m_textures.size(); i++) {
		VkDescriptorSetAllocateInfo textureAllocInfo {};
		textureAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		textureAllocInfo.descriptorPool = m_descriptorPool;
		textureAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		textureAllocInfo.pSetLayouts = textureLayouts.data();

		if (vkAllocateDescriptorSets(m_device.getLogicalDevice(), &textureAllocInfo,
		                             m_textureDescriptorSets[i].data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate texture descriptor sets!");
		}
	}

	// Populate descriptor sets (describe data that goes in each binding available to shader)
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

		// Add descriptor for each texture
		std::vector<VkDescriptorImageInfo> imageInfos(m_textures.size());

		for (uint32_t textureIdx = 0; textureIdx < m_textures.size(); textureIdx++) {
			VkDescriptorImageInfo imageInfo {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_textures[textureIdx]->getImageView();
			imageInfo.sampler = m_textureSampler;

			imageInfos[textureIdx] = imageInfo;
		}

		// Describe the information to write to each descriptor set
		// Uniforms:
		std::vector<VkWriteDescriptorSet> uniformDescriptorWrites(m_uniformBuffers.size());

		for (uint32_t uniformIdx = 0; uniformIdx < m_uniformBuffers.size(); uniformIdx++) {
			uniformDescriptorWrites[uniformIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uniformDescriptorWrites[uniformIdx].dstSet = m_uniformDescriptorSets[frameIdx];
			uniformDescriptorWrites[uniformIdx].dstBinding = uniformIdx;
			uniformDescriptorWrites[uniformIdx].dstArrayElement = 0;
			uniformDescriptorWrites[uniformIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformDescriptorWrites[uniformIdx].descriptorCount = 1;
			uniformDescriptorWrites[uniformIdx].pBufferInfo = &bufferInfos[uniformIdx];
		}

		// Textures:
		std::vector<VkWriteDescriptorSet> textureDescriptorWrites(m_textures.size());
		for (uint32_t imageIdx = 0; imageIdx < m_textures.size(); imageIdx++) {
			textureDescriptorWrites[imageIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			textureDescriptorWrites[imageIdx].dstSet = m_textureDescriptorSets[imageIdx][frameIdx];
			textureDescriptorWrites[imageIdx].dstBinding = 0;
			textureDescriptorWrites[imageIdx].dstArrayElement = 0;
			textureDescriptorWrites[imageIdx].descriptorType =
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureDescriptorWrites[imageIdx].descriptorCount = 1;
			textureDescriptorWrites[imageIdx].pImageInfo = &imageInfos[imageIdx];
		}

		// Write to descriptor sets
		// Uniforms
		vkUpdateDescriptorSets(m_device.getLogicalDevice(),
		                       static_cast<uint32_t>(uniformDescriptorWrites.size()),
		                       uniformDescriptorWrites.data(), 0, nullptr);
		// Textures
		vkUpdateDescriptorSets(m_device.getLogicalDevice(),
		                       static_cast<uint32_t>(textureDescriptorWrites.size()),
		                       textureDescriptorWrites.data(), 0, nullptr);
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
