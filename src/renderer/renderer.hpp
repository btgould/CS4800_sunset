#pragma once

#include "bootstrap/device.hpp"
#include "bootstrap/pipeline.hpp"
#include "renderer/VertexBuffer.hpp"
#include "renderer/IndexBuffer.hpp"
#include "renderer/vertex_array.hpp"
#include "bootstrap/swapchain.hpp"
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
	/* The list attributes each vertex has */
	VertexArray m_vertexArray;

	/* The swapchain the render images to */
	VulkanSwapChain m_swapChain;

	/* The graphics pipeline used to render */
	VulkanPipeline m_pipeline;

	/* Device to execute the rendering on */
	VulkanDevice& m_device;

	/* Buffer holding all the drawing commands for the current frame */
	VkCommandBuffer m_commandBuffer;

	/* Index of the frame in flight being rendered */
	uint32_t m_currentFrame = 0;

	/* Index of the image in the image in the swapchain being rendered to */
	uint32_t m_imageIndex = 0;
};
