#pragma once

#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

#include "bootstrap/device.hpp"
#include "bootstrap/pipeline.hpp"
#include "bootstrap/swapchain.hpp"

#include "renderer/model.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"
#include "renderer/vertex_array.hpp"

struct LightSource {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(4) float ambientStrength;
	alignas(4) float diffuseStrength;
};

struct Cloud {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 scale;
};

class VulkanRenderer {
  public:
	VulkanRenderer(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window);
	~VulkanRenderer();

	VulkanRenderer(const VulkanRenderer&) = delete;
	VulkanRenderer& operator=(const VulkanRenderer&) = delete;

	inline float getAspectRatio() const { return m_swapChain.getAspectRatio(); }

  public:
	void beginScene();
	void draw(VertexBuffer& vertices, IndexBuffer& indices);
	void draw(Model& model);
	void endScene();

	void updateUniform(std::string name, void* data);
	void updatePushConstant(const std::string& name, const void* data);

  private:
	/* The list attributes each vertex has */
	VertexArray m_vertexArray;

	/* The swapchain the render images to */
	VulkanSwapChain m_swapChain;

	/* Device to execute the rendering on */
	VulkanDevice& m_device;

	/* The graphics pipeline used to render */
	VulkanPipeline m_pipeline;

	std::map<std::string, uint32_t> m_uniformIDs;
	std::map<std::string, uint32_t> m_pushConstantIDs;

	/* Buffer holding all the drawing commands for the current frame */
	VkCommandBuffer m_commandBuffer;

	/* Index of the frame in flight being rendered */
	uint32_t m_currentFrame = 0;

	/* Index of the image in the image in the swapchain being rendered to */
	uint32_t m_imageIndex = 0;
};
