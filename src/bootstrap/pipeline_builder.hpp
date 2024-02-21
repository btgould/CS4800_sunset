#pragma once

#include "bootstrap/device.hpp"
#include "bootstrap/shader.hpp"
#include "bootstrap/swapchain.hpp"
#include "pipeline.hpp"
#include "util/memory.hpp"
#include "vertex_array.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

class PipelineBuilder {
  public:
	PipelineBuilder(VulkanDevice& device, const VulkanSwapChain& swapchain);
	~PipelineBuilder();

	PipelineBuilder(const PipelineBuilder&) = delete;
	PipelineBuilder& operator=(const PipelineBuilder&) = delete;

	/**
	 * @brief Creates a new graphics pipeline that expects the given inputs / resources
	 *
	 * @param vertexArray Describes the layout of vertex data rendered with this model
	 * @param pushConstants List of push constants needed by this pipeline
	 *  FIXME: I think I can only use one push constant
	 * @param uniforms List of uniforms needed by this pipeline
	 * @param textures List of Textures available to this pipeline
	 *
	 * @return A VulkanPipeline to render objects with the given structure
	 */
	VulkanPipeline buildPipeline(VertexArray vertexArray, Shader shader,
	                             const std::vector<const Ref<Texture>>& textures);

  private:
	VulkanDevice& m_device;
	const VulkanSwapChain& m_swapChain;
};
