#include "renderer.hpp"

#include "renderer/vertex_array.hpp"
#include "util/profiler.hpp"
#include "util/constants.hpp"
#include <stdexcept>

VulkanRenderer::VulkanRenderer(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window)
	: m_swapChain(instance, device, window), m_device(device),
	  m_texture("res/texture/texture.jpg", m_device),
	  m_pipeline(VulkanPipeline(device, m_swapChain, m_texture.getImageView())) {
	// Configure and create pipeline
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 2}); // pos
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}); // color
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 2}); // uv
	m_pipeline.setVertexArray(m_vertexArray);

	m_uniformIDs["MVP"] = m_pipeline.pushUniform(VK_SHADER_STAGE_VERTEX_BIT, sizeof(MVP));

	m_pipeline.create();
}

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

	// Draw
	vkCmdDrawIndexed(m_commandBuffer, indexBuffer.size(), 1, 0, 0, 0);
}

void VulkanRenderer::endScene() {
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

void VulkanRenderer::updateUniform(std::string name, void* data) {
	auto uniformID = m_uniformIDs.find(name);

	if (uniformID == m_uniformIDs.end()) {
		// uniform name was not found
		throw std::runtime_error("Unrecognized uniform name");
	}

	m_pipeline.writeUniform(uniformID->second, data, m_currentFrame);
}
