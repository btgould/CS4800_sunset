#include "model.hpp"

#include "application/application.hpp"
#include "bootstrap/device.hpp"
#include "renderer/texture.hpp"
#include "renderer/vertex_buffer.hpp"
#include "util/memory.hpp"
#include "util/log.hpp"

#include <tiny_obj_loader.h>
#include <stdexcept>
#include <unordered_map>

Model::Model(Ref<VulkanDevice> device, const std::string& modelPath, Ref<Texture> tex,
             Ref<Shader> shader)
	: m_texture(tex), m_shader(shader) {
	// Load model data
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices {};
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// TODO: I think I can generalize this if I pass a VertexArray to the Model constructor.
	// I will have to add a "name" field to the VertexArray struct, and do some funky math with
	// offsetof. However, this should help with not loading unnecessary data and with checking
	// pipeline / model compatibility. VertexBuffer class is already general enough
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex {};

			vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
			              attrib.vertices[3 * index.vertex_index + 1],
			              attrib.vertices[3 * index.vertex_index + 2]};

			vertex.normal = {attrib.normals[3 * index.vertex_index + 0],
			                 attrib.normals[3 * index.vertex_index + 1],
			                 attrib.normals[3 * index.vertex_index + 2]};

			vertex.color = {1.0f, 1.0f, 1.0f};
			vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
			                   1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	m_vertices =
		CreateScopedRef<VertexBuffer>(device, vertices.data(), sizeof(Vertex), vertices.size());
	m_indices = CreateScopedRef<IndexBuffer>(device, indices);
}

void Model::bind(VkCommandBuffer commandBuffer) {
	m_vertices->bind(commandBuffer);
	m_indices->bind(commandBuffer);
}
