#include "renderer.hpp"

#include "util/profiler.hpp"
#include "util/constants.hpp"

VulkanRenderer::VulkanRenderer(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window)
	: m_swapChain(instance, device, window),
	  m_pipeline(device, this->getDefaultBindingDescription(),
                 this->getDefaultAttributeDescriptions(), m_swapChain.getRenderPass(),
                 m_swapChain.getExtent()),
	  m_device(device) {}

VulkanRenderer::~VulkanRenderer() {}

void VulkanRenderer::beginScene() {
	PROFILE_FUNC();

	// Get image from swap chain
	auto imageIndexOpt = m_swapChain.aquireNextFrame(m_currentFrame);
	if (!imageIndexOpt.has_value()) {
		// Swap chain is recreating, wait until next frame
		return;
	}
	m_imageIndex = imageIndexOpt.value();

	// Prepare to record draw commands
	m_commandBuffer = m_device.getFrameCommandBuffer(m_currentFrame);

	// Start recording
	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;                  // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// Start render pass
	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapChain.getRenderPass();
	renderPassInfo.framebuffer = m_swapChain.getFramebuffer(m_imageIndex);
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_swapChain.getExtent();
	VkClearValue clearColor = {{{1.0f, 0.0f, 1.0f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;
	vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind pipeline
	m_pipeline.bind(m_commandBuffer);
}

void VulkanRenderer::draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
	vertexBuffer.bind(m_commandBuffer);
	indexBuffer.bind(m_commandBuffer);

	m_pipeline.bindDescriptorSets(m_commandBuffer, m_currentFrame);

	// Draw (TODO: I don't understand how this is enough information)
	vkCmdDrawIndexed(m_commandBuffer, indexBuffer.size(), 1, 0, 0, 0);
}

void VulkanRenderer::endScene() {
	// Update uniforms
	// FIXME: This really should be put somewhere else, but will do here for now
	m_pipeline.updateUniformBuffer(m_currentFrame);

	// End render pass, stop recording
	vkCmdEndRenderPass(m_commandBuffer);
	if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	// submit drawing to GPU queue
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // don't color attachment until image is
	                                                    // available
	m_swapChain.submit(m_commandBuffer, waitStages, m_currentFrame);

	// Present rendered image to screen
	m_swapChain.present(m_imageIndex, m_currentFrame);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkVertexInputBindingDescription VulkanRenderer::getDefaultBindingDescription() {
	VkVertexInputBindingDescription bindingDescription {};

	bindingDescription.binding = 0; // NOTE: not exactly sure what a "binding" is here
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> VulkanRenderer::getDefaultAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};

	// Declare position attribute
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	// Declare color attribute
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}
