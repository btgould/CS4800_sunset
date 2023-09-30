#pragma once

#include "device.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

class IndexBuffer {
  public:
	IndexBuffer(VulkanDevice& device, const std::vector<uint16_t>& indices);
	~IndexBuffer();

	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(IndexBuffer&&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	inline uint32_t size() const { return m_indices.size(); }

  private:
	VulkanDevice& m_device;

	const std::vector<uint16_t> m_indices;

	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};
