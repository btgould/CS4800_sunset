#pragma once

#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

#include "application/camera_controller.hpp"
#include "bootstrap/device.hpp"
#include "bootstrap/instance.hpp"
#include "bootstrap/window.hpp"
#include "renderer/index_buffer.hpp"
#include "renderer/vertex_buffer.hpp"
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

class Application {

  public:
	inline static Application& get() { return *s_instance; }
	Application();
	~Application() = default;

  public:
	void run();
	inline const GLFWWindow& getWindow() const { return m_window; }

  private:
	static Application* s_instance;

	GLFWWindow m_window;
	VulkanInstance m_instance;
	VulkanDevice m_device;
	VulkanRenderer m_renderer;
	Camera m_camera;
	CameraController m_camController;

	glm::mat4 m_modelTranslation, m_modelRotation, m_modelScale;

	double m_time;
};
