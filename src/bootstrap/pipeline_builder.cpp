#include "pipeline_builder.hpp"

PipelineBuilder::PipelineBuilder(Ref<VulkanDevice> device, const Ref<VulkanSwapChain> swapchain)
	: m_device(device), m_swapChain(swapchain) {}

PipelineBuilder::~PipelineBuilder() {}

Ref<VulkanPipeline> PipelineBuilder::buildPipeline(VertexArray vertexArray,
                                                   const Ref<Shader> shader,
                                                   const std::vector<Ref<Texture>>& textures,
                                                   bool isPostProcessing) {

	// TODO: I really want the pipeline constructor to be private, and this to be a friend class.
	// However, the CreateRef wraps it in a way that friend doesn't work.
	auto pipeline = CreateRef<VulkanPipeline>(m_device, m_swapChain);

	pipeline->setVertexArray(vertexArray);
	pipeline->setShader(shader);
	pipeline->initializeTextures(textures);
	pipeline->isPostProcessing(isPostProcessing);

	pipeline->create();

	return pipeline;
}
