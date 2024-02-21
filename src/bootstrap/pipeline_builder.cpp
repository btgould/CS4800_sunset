#include "pipeline_builder.hpp"

PipelineBuilder::PipelineBuilder(VulkanDevice& device, const VulkanSwapChain& swapchain)
	: m_device(device), m_swapChain(swapchain) {}
PipelineBuilder::~PipelineBuilder() {}

VulkanPipeline
PipelineBuilder::buildPipeline(VertexArray vertexArray,
                               const std::vector<const PipelineDescriptor>& pushConstants,
                               const std::vector<const PipelineDescriptor>& uniforms,
                               const std::vector<const Ref<Texture>>& textures) {

	return VulkanPipeline(m_device, m_swapChain);
}
