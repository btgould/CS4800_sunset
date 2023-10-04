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
	VulkanPipeline(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	void drawFrame();
	void flush();

  private: // core interface
	void createGraphicsPipeline();
	void createUniformBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);

  private: // helper
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	VulkanDevice& m_device;

	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;

	VulkanSwapChain m_swapChain;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	std::vector<VkDescriptorSet> m_descriptorSets;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

	uint32_t m_currentFrame = 0;
};
