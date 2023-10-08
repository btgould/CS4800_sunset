#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "bootstrap/device.hpp"

/**
 * @class VertexBuffer
 * @brief Represents a list of Vertices, each of which can have an arbitrary list of attributes.
 *
 */
class VertexBuffer {
  public:
	/**
	 * @brief Creates a new vertex buffer on the given device to hold the given vertex data
	 *
	 * @param device A VulkanDevice to create the buffer on
	 * @param vertices Array of vertices, each of which must have the same set and order of
	 * attributes
	 */
	VertexBuffer(VulkanDevice& device, void* data, uint32_t vertexSize, uint32_t count);

	/**
	 * @brief Frees memory allocated on the GPU for this buffer
	 */
	~VertexBuffer();

	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer operator=(const VertexBuffer&) = delete;

	/**
	 * @brief "Activates" this buffer, causing it to be used for the next draw call
	 *
	 * @param commandBuffer The command buffer to record to
	 */
	void bind(VkCommandBuffer commandBuffer);

	inline uint32_t size() const { return m_count; }

  private:
	/* Device to store this buffer on */
	VulkanDevice& m_device;

	void* m_data;
	uint32_t m_vertexSize;
	uint32_t m_count;

	/* Buffer object to store vertex data */
	VkBuffer m_vertexBuffer;

	/* Location of GPU memory used to store vertex data */
	VkDeviceMemory m_vertexBufferMemory;
};
