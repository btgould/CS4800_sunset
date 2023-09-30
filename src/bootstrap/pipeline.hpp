#pragma once

#include <vulkan/vulkan_core.h>
#include <fstream>
#include <vector>

#include "instance.hpp"

#include "renderer/VertexBuffer.hpp"

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

class VulkanPipeline {
  public:
	VulkanPipeline(VulkanInstance& instance);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	void drawFrame();
	void flush();

  private: // core interface
	void createGraphicsPipeline();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  private: // helper
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	VulkanInstance& m_instance;

	VertexBuffer m_vertexBuffer;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

	uint32_t m_currentFrame = 0;
};
