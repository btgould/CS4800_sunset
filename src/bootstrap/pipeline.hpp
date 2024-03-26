#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>

#include "bootstrap/shader.hpp"
#include "renderer/model.hpp"
#include "renderer/texture.hpp"
#include "util/constants.hpp"

#include "device.hpp"

#include "vertex_array.hpp"
#include "swapchain.hpp"
#include "util/memory.hpp"

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

/**
 * @class VulkanPipeline
 * @brief Class describing a rendering pipeline to be used for a specific set of resources
 *
 * Cannot be instantiated directly, instead use the PipelineBuilder class
 *
 */
class VulkanPipeline {
	friend class PipelineBuilder;

  private:
  public:
	VulkanPipeline(Ref<VulkanDevice> device, const Ref<VulkanSwapChain> swapChain);
	~VulkanPipeline();

  public:
	void writeUniform(const std::string& name, void* data, uint32_t currentFrame);
	inline void setActiveTexture(const Ref<Texture> tex) { m_activeTex = tex; }
	void writePushConstant(VkCommandBuffer commandBuffer, const std::string& name, const void* data,
	                       uint32_t currentFrame);

	void bind(VkCommandBuffer commandBuffer);
	void bindTexture(Ref<Texture> tex);
	void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	bool canRender(const Model& model);

	// HACK: this exists solely to satisfy ImGui
	VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }

  private: // core interface
	/**
	 * @brief Creates the pipeline object on the GPU
	 *
	 * After this creation, no pipeline configuration can be changed. If a change is needed, the
	 * pipeline must be recreated from scratch.
	 */
	void create();

	void setVertexArray(const VertexArray& vertexArray);
	void setShader(const Ref<Shader> shader);
	uint32_t setPushConstant(const PipelineDescriptor& pushConstant);
	uint32_t pushUniform(const PipelineDescriptor& uniform);
	void pushTexture(const Ref<Texture> tex);

	void createGraphicsPipeline(VkVertexInputBindingDescription bindingDesc,
	                            std::vector<VkVertexInputAttributeDescription> attrDesc,
	                            VkRenderPass renderPass);
	void createTextureSampler();

  private: // helper
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	PipelineConfigInfo defaultPipelineConfigInfo();

  private:
	Ref<VulkanDevice> m_device;
	const Ref<VulkanSwapChain> m_swapChain;

	VkVertexInputBindingDescription m_vertexAttrBindings;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttr;
	VkSampler m_textureSampler;

	template <typename T> using Frames = std::array<T, MAX_FRAMES_IN_FLIGHT>;
	std::array<VkDescriptorSet, 2> m_activeDescriptorSets;

	Ref<Shader> m_shader;

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
	std::map<std::string, uint32_t> m_uniformIDs;
	std::map<std::string, uint32_t>
		m_pushConstantIDs; // HACK: I can really only have one push
	                       // constant, having a map is redundant. See comment in writePushConstant
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
