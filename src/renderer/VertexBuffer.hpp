#include <array>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "bootstrap/device.hpp"

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
};

class VertexBuffer {
  public:
	VertexBuffer(VulkanDevice& device, const std::vector<Vertex>& vertices);
	~VertexBuffer();

	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer operator=(const VertexBuffer&) = delete;

	// TODO: these are very hardcoded for now, will want to change to make a renderer
	VkVertexInputBindingDescription getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	void bind(VkCommandBuffer commandBuffer);

	inline uint32_t size() const { return m_vertices.size(); }

  private:
	std::vector<Vertex> m_vertices;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VulkanDevice& m_device;
};
