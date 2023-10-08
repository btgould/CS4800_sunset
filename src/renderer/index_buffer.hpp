#pragma once

#include "bootstrap/device.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

/**
 * @class IndexBuffer
 * @brief Keeps track of the order to render vertices in.
 *
 */
class IndexBuffer {
  public:
	/**
	 * @brief Creates a new index buffer on the given device to hold the given indices
	 *
	 * @param device A VulkanDevice to create the buffer on
	 * @param indices The "connect the dots" ordering to render the vertices in
	 */
	IndexBuffer(VulkanDevice& device, const std::vector<uint32_t>& indices);

	/**
	 * @brief Frees memory allocated on the GPU for this buffer
	 */
	~IndexBuffer();

	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(IndexBuffer&&) = delete;

	/**
	 * @brief "Activates" this index buffer, causing its indices to be used for the next draw call
	 *
	 * @param commandBuffer The command buffer to record the binding to
	 */
	void bind(VkCommandBuffer commandBuffer);

	/**
	 * @return Gets the number of indices in this index buffer
	 */
	inline uint32_t size() const { return m_indices.size(); }

  private:
	VulkanDevice& m_device;

	const std::vector<uint32_t> m_indices;

	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};
