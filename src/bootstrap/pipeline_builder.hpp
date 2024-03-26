#pragma once

#include "bootstrap/device.hpp"
#include "bootstrap/shader.hpp"
#include "bootstrap/swapchain.hpp"
#include "pipeline.hpp"
#include "util/memory.hpp"
#include "vertex_array.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

/**
 * @class PipelineBuilder
 * @brief Instantiates VulkanPipeline objects, performing all needed setup so that the obtained objects are ready to use
 *
 */
class PipelineBuilder {
  public:
	PipelineBuilder(Ref<VulkanDevice> device, const Ref<VulkanSwapChain> swapchain);
	~PipelineBuilder();

	PipelineBuilder(const PipelineBuilder&) = delete;

	/**
	 * @brief Creates a new graphics pipeline that expects the given inputs / resources
	 *
	 * @param vertexArray Describes the layout of vertex data rendered with this model
	 * @param shader The shader used by this pipeline to render data
	 * @param textures List of Textures available to this pipeline
	 *
	 * @return A VulkanPipeline to render objects with the given structure
	 */
	Ref<VulkanPipeline> buildPipeline(VertexArray vertexArray, const Ref<Shader> shader,
	                                  const std::vector<Ref<Texture>>& textures);

  private:
	Ref<VulkanDevice> m_device;
	const Ref<VulkanSwapChain> m_swapChain;
};
