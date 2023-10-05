#pragma once

#include <vulkan/vulkan_core.h>
#include <fstream>
#include <vector>

#include "device.hpp"
#include "instance.hpp"

#include "renderer/IndexBuffer.hpp"
#include "renderer/VertexBuffer.hpp"
#include "swapchain.hpp"
#include "window.hpp"

struct PipelineConfigInfo {
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
};

struct MVP {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class VulkanPipeline {
  public:
	VulkanPipeline(VulkanDevice& device, VkVertexInputBindingDescription bindingDesc,
	               std::vector<VkVertexInputAttributeDescription> attrDesc, VkRenderPass renderPass,
	               VkExtent2D extant, VkImageView imageView);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void updateUniformBuffer(uint32_t currentImage);

  private: // core interface
	void createGraphicsPipeline(VkVertexInputBindingDescription bindingDesc,
	                            std::vector<VkVertexInputAttributeDescription> attrDesc,
	                            VkRenderPass renderPass);
	void createUniformBuffers();
	void createTextureSampler();

  private: // helper
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets(VkImageView imageView);
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	VulkanDevice& m_device;

	/* Buffer to store uniforms data in */
	std::vector<VkBuffer> m_uniformBuffers;
	/* Memory on GPU to store uniform buffers */
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	/* CPU address linked to location of uniforms on GPU */
	std::vector<void*> m_uniformBuffersMapped;

	VkSampler m_textureSampler;

	/* Pool to allocate descriptor sets from */
	VkDescriptorPool m_descriptorPool;
	/* A list of buffers used by shaders. I use this to upload uniforms. */
	VkDescriptorSetLayout m_descriptorSetLayout;
	/* Describes where to get uniform data, and how the GPU should use it */
	std::vector<VkDescriptorSet> m_descriptorSets;

	VkExtent2D m_extant;

	/* A list of descriptor layouts, describing dynamic resources used by pipeline */
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;
};
