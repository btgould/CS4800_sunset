#pragma once

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

enum VertexAtrributeType {
	VERTEX_ATTRIB_TYPE_F32,
};

struct VertexAtrribute {
	VertexAtrributeType type;
	uint32_t count;

	uint32_t getSize() const { return getTypeSize(type) * count; }
	VkFormat getFormat() const {
		switch (type) {
		case VERTEX_ATTRIB_TYPE_F32:
			switch (count) {
			case 1:
				return VK_FORMAT_R32_SFLOAT;
			case 2:
				return VK_FORMAT_R32G32_SFLOAT;
			case 3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case 4:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			default:
				throw std::runtime_error("Invalid count for vertex attribute");
			}
		default:
			throw std::runtime_error("Unrecognized vertex attribute type!");
		}
	}

  private:
	uint32_t getTypeSize(VertexAtrributeType type) const {
		switch (type) {
		case VERTEX_ATTRIB_TYPE_F32:
			return 4;
		}

		throw std::runtime_error("Unrecognized vertex attribute type!");
	}
};

class VertexArray {
  public:
	VertexArray();
	~VertexArray();

	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;

  public:
	void push(VertexAtrribute attr);

	VkVertexInputBindingDescription getBindingDescription() const;
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;

  private:
	std::vector<VertexAtrribute> m_attribs;
};
