#pragma once

#include <string>
#include <vulkan/vulkan_core.h>

#include "bootstrap/device.hpp"
#include "bootstrap/pipeline.hpp"
#include "bootstrap/pipeline_builder.hpp"
#include "bootstrap/swapchain.hpp"

#include "renderer/model.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"

class VulkanRenderer {
  public:
	VulkanRenderer(Ref<VulkanInstance> instance, Ref<VulkanDevice> device, Ref<GLFWWindow> window);
	~VulkanRenderer();

	VulkanRenderer(const VulkanRenderer&) = delete;

	inline float getAspectRatio() const { return m_swapChain->getAspectRatio(); }

  public:
	void beginScene();
	void draw(VertexBuffer& vertices, IndexBuffer& indices);
	void draw(Model& model);
	void endScene();

	void updateUniform(std::string name, void* data);
	void updatePushConstant(const std::string& name, const void* data);

  private:
	void findOrBuildPipeline(const Model& model);

  private:
	/* The swapchain the render images to */
	Ref<VulkanSwapChain> m_swapChain;

	/* Device to execute the rendering on */
	Ref<VulkanDevice> m_device;

	PipelineBuilder m_pipelineBuilder;
	std::vector<Ref<VulkanPipeline>> m_pipelines;
	Ref<VulkanPipeline> m_activePipeline;

	const VertexArray m_defaultVA;
	const std::vector<Ref<Texture>> m_textures;

	/* Buffer holding all the drawing commands for the current frame */
	VkCommandBuffer m_commandBuffer;

	/* Index of the frame in flight being rendered */
	uint32_t m_currentFrame = 0;

	/* Index of the image in the image in the swapchain being rendered to */
	uint32_t m_imageIndex = 0;
};
