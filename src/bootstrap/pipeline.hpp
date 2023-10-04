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
	               std::array<VkVertexInputAttributeDescription, 2> attrDesc,
	               VkRenderPass renderPass, VkExtent2D extant);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame);
	void updateUniformBuffer(uint32_t currentImage);

  private: // core interface
	void createGraphicsPipeline(VkVertexInputBindingDescription bindingDesc,
	                            std::array<VkVertexInputAttributeDescription, 2> attrDesc,
	                            VkRenderPass renderPass);
	void createUniformBuffers();

  private: // helper
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	VulkanDevice& m_device;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	std::vector<VkDescriptorSet> m_descriptorSets;

	VkExtent2D m_extant;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;
};
