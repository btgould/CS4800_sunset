#include "pipeline_builder.hpp"

PipelineBuilder::PipelineBuilder(Ref<VulkanDevice> device, const Ref<VulkanSwapChain> swapchain)
	: m_device(device), m_swapChain(swapchain) {}

PipelineBuilder::~PipelineBuilder() {}

Ref<VulkanPipeline> PipelineBuilder::buildPipeline(VertexArray vertexArray,
                                                   const Ref<Shader> shader,
                                                   const std::vector<Ref<Texture>>& textures) {

	auto pipeline = CreateRef<VulkanPipeline>(m_device, m_swapChain);

	pipeline->setVertexArray(vertexArray);
	pipeline->setShader(shader);
	for (const auto texture : textures) {
		pipeline->pushTexture(texture);
	}

	pipeline->create();

	return pipeline;
}
