#include "vertex_buffer.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

VertexBuffer::VertexBuffer(VulkanDevice& device, void* data, uint32_t vertexSize, uint32_t count)
	: m_device(device), m_data(data), m_vertexSize(vertexSize), m_count(count) {
	VkDeviceSize bufferSize = m_count * m_vertexSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                      stagingBuffer, stagingBufferMemory);

	// Copy data from CPU to GPU
	vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_data, (size_t) bufferSize);
	vkUnmapMemory(device.getLogicalDevice(),
	              stagingBufferMemory); // We can immediately unmap here because of the
	                                    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property. Otherwise,
	                                    // we would have to explicitly flush before unmap

	// Move from staging buffer to (optimally formatted) vertex buffer
	m_device.createBuffer(
		bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

	m_device.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

	// cleanup staging buffer
	vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);
}

VertexBuffer::~VertexBuffer() {
	vkDestroyBuffer(m_device.getLogicalDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_device.getLogicalDevice(), m_vertexBufferMemory, nullptr);
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) {
	VkBuffer vertexBuffers[] = {m_vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}
