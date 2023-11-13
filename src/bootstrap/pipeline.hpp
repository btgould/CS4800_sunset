#pragma once

#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include <fstream>
#include <vector>

#include "renderer/texture.hpp"
#include "util/constants.hpp"

#include "device.hpp"
#include "instance.hpp"

#include "renderer/index_buffer.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/vertex_array.hpp"
#include "swapchain.hpp"
#include "util/memory.hpp"
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

class VulkanPipeline {
  public:
	VulkanPipeline(VulkanDevice& device, const VulkanSwapChain& swapChain);
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

	void pushTexture(const Ref<Texture> tex);
	inline void setActiveTexture(const Ref<Texture> tex) { m_activeTex = tex; }

	uint32_t pushPushConstant(VkShaderStageFlags stage, uint32_t size);
	void writePushConstant(VkCommandBuffer commandBuffer, uint32_t pushConstantId, const void* data,
	                       uint32_t currentFrame);

	void bind(VkCommandBuffer commandBuffer);
	void bindTexture(Ref<Texture> tex);
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
	void createDescriptorSets();
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	VulkanDevice& m_device;
	const VulkanSwapChain& m_swapChain;

	VkVertexInputBindingDescription m_vertexAttrBindings;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttr;
	VkSampler m_textureSampler;

	template <typename T> using Frames = std::array<T, MAX_FRAMES_IN_FLIGHT>;
	std::array<VkDescriptorSet, 2> m_activeDescriptorSets;

	// Texture resources
	Ref<Texture> m_activeTex;
	std::vector<Ref<Texture>> m_textures;
	std::unordered_map<Ref<Texture>, uint32_t>
		m_textureIdx; // TODO: using two DS like this is clunky
	/* list of structs, each describing a texture this pipeline makes available to shaders */
	std::vector<VkDescriptorSetLayoutBinding> m_textureBindings;
	VkDescriptorSetLayout m_textureLayout;
	std::vector<Frames<VkDescriptorSet>> m_textureDescriptorSets;

	// Uniform resources
	std::vector<uint32_t> m_uniformSizes;
	/* Buffer to store uniforms data in */
	std::vector<Frames<VkBuffer>> m_uniformBuffers;
	/* Memory on GPU to store uniform buffers */
	std::vector<Frames<VkDeviceMemory>> m_uniformBuffersMemory;
	/* CPU address linked to location of uniforms on GPU */
	std::vector<Frames<void*>> m_uniformBuffersMapped;
	/* list of structs, each describing a uniform this pipeline makes available to shaders */
	std::vector<VkDescriptorSetLayoutBinding> m_uniformBindings;
	/* A list of buffers used by shaders. I use this to upload uniforms. */
	VkDescriptorSetLayout m_uniformLayout;
	/* Describes where to get uniform data, and how the GPU should use it */
	Frames<VkDescriptorSet> m_uniformDescriptorSets;

	/* Pool to allocate descriptor sets from */
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorPoolSize> m_poolSizes;
	/* A list of descriptor layouts, describing dynamic resources used by pipeline */
	VkPipelineLayout m_pipelineLayout;

	// Push constant resources
	std::vector<VkPushConstantRange> m_pushConstants;
	std::vector<uint32_t> m_pushConstantSizes;
	uint32_t m_pushConstantOffset = 0;

	VkPipeline m_pipeline;
};
