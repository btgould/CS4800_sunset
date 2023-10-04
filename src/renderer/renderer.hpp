#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "renderer/VertexBuffer.hpp"
#include "renderer/IndexBuffer.hpp"
#include "swapchain.hpp"
#include <vulkan/vulkan_core.h>

class VulkanRenderer {
  public:
	VulkanRenderer(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window);
	~VulkanRenderer();

	VulkanRenderer(const VulkanRenderer&) = delete;
	VulkanRenderer& operator=(const VulkanRenderer&) = delete;

  public:
	void beginScene();
	void draw(VertexBuffer& vertices, IndexBuffer& indices);
	void endScene();

  private:
	// TODO: these are very hardcoded for now, will want to change to allow arbitrary layouts
	VkVertexInputBindingDescription getDefaultBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> getDefaultAttributeDescriptions();

  private:
	/* The swapchain the render images to */
	VulkanSwapChain m_swapChain;

	/* The graphics pipeline used to render */
	VulkanPipeline m_pipeline;

	/* Device to execute the rendering on */
	VulkanDevice& m_device;

	/* Buffer holding all the drawing commands for the current frame */
	VkCommandBuffer m_commandBuffer;

	// TODO: Figure out what makes these two things distinct!
	uint32_t m_currentFrame = 0;
	uint32_t m_imageIndex = 0;
};
