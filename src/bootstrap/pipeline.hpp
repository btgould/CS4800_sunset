#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>

#include "renderer/shader.hpp"
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

template <typename T> using Frames = std::array<T, MAX_FRAMES_IN_FLIGHT>;

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
	void setAvailableTextures(const std::vector<Ref<Texture>>& textures);
	void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	bool canRender(const Model& model);
	void inline isPostProcessing(bool isPostProcessing) { m_isPostProcessing = isPostProcessing; }

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
	/**
	 * @brief Defines the shader, uniforms, and push constant used by this pipeline.
	 *
	 * @param shader The shader object for this pipeline to use
	 */
	void setShader(const Ref<Shader> shader);
	uint32_t setPushConstant(const PipelineDescriptor& pushConstant);
	void setUniforms(const std::vector<PipelineDescriptor>& uniforms);
	inline void initializeTextures(const std::vector<Ref<Texture>>& textures) {
		m_textures = textures;
	}

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
	bool m_isPostProcessing;

	std::array<VkDescriptorSet, 2> m_activeDescriptorSets;

	Ref<Shader> m_shader;

	// Texture resources
	Ref<Texture> m_activeTex;
	std::vector<Ref<Texture>> m_textures;
	std::unordered_map<Ref<Texture>, uint32_t>
		m_textureIdx; // TODO: using two DS like this is clunky
	// Descriptor set layout for textures: hardcoded to expect one albedo, one normal
	VkDescriptorSetLayout m_textureLayout;
	// List of descriptor sets (each with m_textureLayout layout). One descriptor set for each frame
	// in flight and albedo / normal pair we render
	std::vector<Frames<VkDescriptorSet>> m_textureDescriptorSets;

	// Uniform resources
	std::map<std::string, uint32_t>
		m_pushConstantIDs; // HACK: I can really only have one push
	                       // constant, having a map is redundant. See comment in writePushConstant
	std::map<std::string, uint32_t> m_uniformIds;
	std::vector<uint32_t> m_uniformSizes;
	std::vector<uint32_t> m_uniformOffsets;
	/* Buffer to store uniforms data in */
	Frames<VkBuffer> m_uniformBuffers;
	/* Memory on GPU to store uniform buffers */
	Frames<VkDeviceMemory> m_uniformBuffersMemory;
	/* CPU address linked to location of uniforms on GPU */
	Frames<void*> m_uniformBuffersMapped;
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
