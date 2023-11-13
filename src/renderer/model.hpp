#pragma once

#include "renderer/transform.hpp"
#include "util/memory.hpp"

#include "bootstrap/device.hpp"

#include "renderer/index_buffer.hpp"
#include "renderer/texture.hpp"
#include "renderer/vertex_buffer.hpp"

#include <string>
#include <vulkan/vulkan_core.h>

class Model {
  public:
	Model(VulkanDevice& device, const std::string& modelPath, Ref<Texture> tex);
	~Model() = default;

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

  public:
	void bind(VkCommandBuffer commandBuffer);
	Ref<Texture> getTexture() { return m_texture; }

	inline uint32_t numIndices() { return m_indices->size(); }
	inline Transform& getTransform() { return m_transform; }

  private:
	ScopedRef<VertexBuffer> m_vertices;
	ScopedRef<IndexBuffer> m_indices;
	Ref<Texture> m_texture;

	Transform m_transform;
};
