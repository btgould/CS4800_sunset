#include <stdexcept>
#include <unordered_map>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

#include "spdlog/common.h"

#include "tiny_obj_loader.h"

#include "util/log.hpp"
#include "util/profiler.hpp"
#include "bootstrap/device.hpp"
#include "bootstrap/pipeline.hpp"
#include "bootstrap/instance.hpp"
#include "bootstrap/window.hpp"
#include "renderer/index_buffer.hpp"
#include "renderer/renderer.hpp"
#include "renderer/camera.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template <> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
			       (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
} // namespace std

class HelloTriangleApplication {
  public:
	HelloTriangleApplication()
		: m_window(GLFWWindow("Vulkan")), m_instance(m_window), m_device(m_instance),
		  m_renderer(m_instance, m_device, m_window),
		  m_camera(glm::radians(45.0f), m_renderer.getAspectRatio(), glm::vec3(2.0f, 2.0f, 2.0f)) {
		m_modelTranslation = glm::mat4(1.0f);
		m_modelRotation = glm::mat4(1.0f);
		m_modelScale = glm::mat4(1.0f);
	}

	~HelloTriangleApplication() = default;

	void run() {
		const std::string modelPath = "res/model/viking_room.obj";
		const std::string texPath = "res/texture/viking_room.png";

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

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex {};

				vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
				              attrib.vertices[3 * index.vertex_index + 1],
				              attrib.vertices[3 * index.vertex_index + 2]};

				vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
				                   1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
				vertex.color = {1.0f, 1.0f, 1.0f};

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		VertexBuffer modelVB(m_device, vertices.data(), sizeof(Vertex), vertices.size());
		IndexBuffer modelIB(m_device, indices);

		while (!m_window.shouldClose()) {
			m_window.pollEvents();

			m_renderer.beginScene();
			m_renderer.draw(modelVB, modelIB);

			// update uniforms
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float dt =
				std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime)
					.count();

			m_modelRotation =
				glm::rotate(glm::mat4(1.0f), dt * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::mat4 modelTRS = m_modelScale * m_modelRotation * m_modelTranslation;
			m_renderer.updateUniform("modelTRS", &modelTRS);
			glm::mat4 camVP = m_camera.getVP();
			m_renderer.updateUniform("camVP", &camVP);

			m_renderer.endScene();
		}

		m_device.flush();
	}

  private:
	GLFWWindow m_window;
	VulkanInstance m_instance;
	VulkanDevice m_device;
	VulkanRenderer m_renderer;
	Camera m_camera;

	glm::mat4 m_modelTranslation, m_modelRotation, m_modelScale;
};

int main() {
	Log::Init(spdlog::level::info);
	HelloTriangleApplication app;

	try {
		PROFILE_BEGIN_SESSION("Runtime", "Runtime_Profile.json");
		app.run();
		PROFILE_END_SESSION();
	} catch (const std::exception& e) {
		LOG_ERROR("{0}", e.what());
		PROFILE_END_SESSION();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
