#include "vertex_array.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

VertexArray::VertexArray() {}

VertexArray::~VertexArray() {}

void VertexArray::push(VertexAtrribute attr) {
	m_attribs.push_back(attr);
}

VkVertexInputBindingDescription VertexArray::getBindingDescription() const {
	// calculate total size of vertex
	uint32_t totalSize = 0;

	for (const auto& attr : m_attribs) {
		totalSize += attr.getSize();
	}

	// Generate binding desc
	VkVertexInputBindingDescription bindingDescription {};

	bindingDescription.binding = 0; // NOTE: not exactly sure what a "binding" is here
	bindingDescription.stride = totalSize;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VertexArray::getAttributeDescriptions() const {
	std::vector<VkVertexInputAttributeDescription> descList(m_attribs.size());

	uint32_t runningOffset = 0;

	for (uint32_t i = 0; i < m_attribs.size(); i++) {
		VertexAtrribute attr = m_attribs[i];
		VkVertexInputAttributeDescription desc;

		desc.binding = 0;
		desc.location = i;
		desc.format = attr.getFormat();
		desc.offset = runningOffset;

		descList[i] = desc;

		runningOffset += attr.getSize();
	}

	return descList;
}
