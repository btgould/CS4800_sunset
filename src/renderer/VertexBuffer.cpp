#include "VertexBuffer.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

VertexBuffer::VertexBuffer(VulkanDevice& device, const std::vector<Vertex>& vertices)
	: m_device(device), m_vertices(vertices) {
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                      stagingBuffer, stagingBufferMemory);

	// Copy data from CPU to GPU
	void* data;
	vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t) bufferSize);
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

VkVertexInputBindingDescription VertexBuffer::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription {};

	bindingDescription.binding = 0; // NOTE: not exactly sure what a "binding" is here
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> VertexBuffer::getAttributeDescriptions() {
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

void VertexBuffer::bind(VkCommandBuffer commandBuffer) {
	VkBuffer vertexBuffers[] = {m_vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}