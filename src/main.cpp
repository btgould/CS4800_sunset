#include <stdexcept>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "spdlog/common.h"

#include "util/log.hpp"
#include "util/profiler.hpp"
#include "bootstrap/device.hpp"
#include "bootstrap/pipeline.hpp"
#include "bootstrap/instance.hpp"
#include "bootstrap/window.hpp"
#include "renderer/IndexBuffer.hpp"
#include "renderer/renderer.hpp"
#include "renderer/camera.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

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
		VertexBuffer vb1(
			m_device,
			std::vector<Vertex>({{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		                         {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		                         {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		                         {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}})
				.data(),
			sizeof(float) * 8, 4);
		VertexBuffer vb2(
			m_device,
			std::vector<Vertex>({{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		                         {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		                         {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		                         {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}})
				.data(),
			sizeof(float) * 8, 4);
		IndexBuffer ib(m_device, {0, 1, 2, 2, 3, 0});

		while (!m_window.shouldClose()) {
			m_window.pollEvents();

			m_renderer.beginScene();
			m_renderer.draw(vb1, ib);
			m_renderer.draw(vb2, ib);

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
