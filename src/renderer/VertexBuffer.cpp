#include "VertexBuffer.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

VertexBuffer::VertexBuffer(const VulkanDevice& device, const std::vector<Vertex>& vertices)
	: m_device(device), m_vertices(vertices) {
	// Create buffer object
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(m_vertices[0]) * m_vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device.getLogicalDevice(), &bufferInfo, nullptr, &m_vertexBuffer) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	// Allocate memory for buffer on GPU
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getLogicalDevice(), m_vertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_device.findMemoryType(memRequirements.memoryTypeBits,
	                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &m_vertexBufferMemory) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	// Associate vertex buffer with memory on GPU
	// 0 here means offset from start of allocated memory
	vkBindBufferMemory(device.getLogicalDevice(), m_vertexBuffer, m_vertexBufferMemory, 0);

	void* data;
	vkMapMemory(device.getLogicalDevice(), m_vertexBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), (size_t) bufferInfo.size);
	vkUnmapMemory(
		device.getLogicalDevice(),
		m_vertexBufferMemory); // We can immediately unmap here because of the
	                           // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property. Otherwise, we would
	                           // have to explicitly flush before unmap
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

void VertexBuffer::Bind(VkCommandBuffer commandBuffer) {
	VkBuffer vertexBuffers[] = {m_vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}
