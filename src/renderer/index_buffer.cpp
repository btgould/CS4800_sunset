#include "index_buffer.hpp"

#include <cstring>

IndexBuffer::IndexBuffer(Ref<VulkanDevice> device, const std::vector<uint32_t>& indices)
	: m_device(device), m_indices(indices) {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// Initial buffer to hold memory on GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                       stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) bufferSize);
	vkUnmapMemory(device->getLogicalDevice(), stagingBufferMemory);

	// Copy memory to new buffer with optimized memory format
	m_device->createBuffer(bufferSize,
	                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

	m_device->copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

	// Free memory for staging buffer
	vkDestroyBuffer(device->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device->getLogicalDevice(), stagingBufferMemory, nullptr);
}

IndexBuffer::~IndexBuffer() {
	vkDestroyBuffer(m_device->getLogicalDevice(), m_indexBuffer, nullptr);
	vkFreeMemory(m_device->getLogicalDevice(), m_indexBufferMemory, nullptr);
}

void IndexBuffer::bind(VkCommandBuffer commandBuffer) {
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}
