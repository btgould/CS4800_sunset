#pragma once

#include "util/memory.hpp"

#include "bootstrap/device.hpp"

#include "renderer/index_buffer.hpp"
#include "renderer/texture.hpp"
#include "renderer/vertex_buffer.hpp"

#include <string>
#include <vulkan/vulkan_core.h>

class Model {
  public:
	Model(VulkanDevice& device, const std::string& modelPath, const std::string& texPath);
	~Model() = default;

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

  public:
	void bind(VkCommandBuffer commandBuffer);

	inline uint32_t numIndices() { return m_indices->size(); }

  private:
	ScopedRef<VertexBuffer> m_vertices;
	ScopedRef<IndexBuffer> m_indices;
	ScopedRef<Texture> m_texture;
};
