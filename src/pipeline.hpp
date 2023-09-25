#pragma once

#include <vulkan/vulkan_core.h>

#include "instance.hpp"

#include <fstream>
#include <vector>

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
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class VulkanPipeline {
  public:
	VulkanPipeline(VulkanInstance& instance);
	~VulkanPipeline();

  private:
	void createGraphicsPipeline();
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	VulkanInstance m_instance;

	VkPipelineLayout m_pipelineLayout;
};
