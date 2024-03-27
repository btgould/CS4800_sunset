#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

#include "bootstrap/device.hpp"
#include "bootstrap/instance.hpp"
#include "bootstrap/window.hpp"
#include "renderer/renderer.hpp"
#include "renderer/camera.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && normal == other.normal &&
		       texCoord == other.texCoord;
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
	void shutdown();
	inline const Ref<GLFWWindow> getWindow() const { return m_window; }

  private:
	static Application* s_instance;

	Ref<GLFWWindow> m_window;
	Ref<VulkanInstance> m_instance;
	Ref<VulkanDevice> m_device;
	Ref<VulkanRenderer> m_renderer;
	Ref<Camera> m_camera;

	double m_time;
};
