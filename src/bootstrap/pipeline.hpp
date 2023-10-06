#pragma once

#include <vulkan/vulkan_core.h>
#include <fstream>
#include <vector>

#include "util/constants.hpp"

#include "device.hpp"
#include "instance.hpp"

#include "renderer/IndexBuffer.hpp"
#include "renderer/VertexBuffer.hpp"
#include "renderer/vertex_array.hpp"
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
	VulkanPipeline(VulkanDevice& device, const VulkanSwapChain& swapChain, VkImageView imageView);
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

  public:
	/**
	 * @brief Creates the pipeline object on the GPU
	 *
	 * After this creation, no pipeline configuration can be changed. If a change is needed, the
	 * pipeline must be recreated from scratch.
	 */
	void create();

	void setVertexArray(const VertexArray& vertexArray);
	uint32_t pushUniform(VkShaderStageFlags stage, uint32_t size);
	void writeUniform(uint32_t uniformID, void* data, uint32_t currentFrame);

	void bind(VkCommandBuffer commandBuffer);
	void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame);

  private: // core interface
	void createGraphicsPipeline(VkVertexInputBindingDescription bindingDesc,
	                            std::vector<VkVertexInputAttributeDescription> attrDesc,
	                            VkRenderPass renderPass);
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
	const VulkanSwapChain& m_swapChain;

	VkVertexInputBindingDescription m_vertexAttrBindings;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttr;

	// TODO: I'd really like to find a way to get rid of this
	VkImageView m_imageView;

	template <typename T> using Frames = std::array<T, MAX_FRAMES_IN_FLIGHT>;

	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	std::vector<uint32_t> m_uniformSizes;
	std::vector<VkDescriptorPoolSize> m_poolSizes;

	/* Buffer to store uniforms data in */
	std::vector<Frames<VkBuffer>> m_uniformBuffers;
	/* Memory on GPU to store uniform buffers */
	std::vector<Frames<VkDeviceMemory>> m_uniformBuffersMemory;
	/* CPU address linked to location of uniforms on GPU */
	std::vector<Frames<void*>> m_uniformBuffersMapped;

	VkSampler m_textureSampler; // TODO: I want a pool of texture samplers, statically owned by
	                            // texture class

	/* Pool to allocate descriptor sets from */
	VkDescriptorPool m_descriptorPool;
	/* A list of buffers used by shaders. I use this to upload uniforms. */
	VkDescriptorSetLayout m_descriptorSetLayout;
	/* Describes where to get uniform data, and how the GPU should use it */
	std::vector<VkDescriptorSet> m_descriptorSets;
	/* A list of descriptor layouts, describing dynamic resources used by pipeline */
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;
};
