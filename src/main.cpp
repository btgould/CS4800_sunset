#include <stdexcept>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "spdlog/common.h"

#include "util/log.hpp"
#include "util/profiler.hpp"
#include "bootstrap/device.hpp"
#include "bootstrap/pipeline.hpp"
#include "bootstrap/instance.hpp"
#include "bootstrap/window.hpp"
#include "renderer/IndexBuffer.hpp"
#include "renderer/renderer.hpp"

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

class HelloTriangleApplication {
  public:
	HelloTriangleApplication()
		: m_window(GLFWWindow("Vulkan")), m_instance(m_window), m_device(m_instance),
		  m_renderer(m_instance, m_device, m_window),
		  m_vertexBuffer(m_device,
	                     std::vector<Vertex>({{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	                                          {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	                                          {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	                                          {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}})
	                         .data(),
	                     sizeof(float) * 7, 4),
		  m_indexBuffer(m_device, {0, 1, 2, 2, 3, 0}) {}

	~HelloTriangleApplication() = default;

	void run() {
		while (!m_window.shouldClose()) {
			m_window.pollEvents();

			m_renderer.beginScene();
			m_renderer.draw(m_vertexBuffer, m_indexBuffer);

			// update uniforms
			// Get current time
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time =
				std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime)
					.count();

			// Calculate transformation based on current time
			// TODO: CAMERA VP should really be abstracted to camera class
			MVP ubo;
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
			                        glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			                       glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj =
				glm::perspective(glm::radians(45.0f), m_renderer.getAspectRatio(), 0.1f, 10.0f);
			ubo.proj[1][1] *= -1; // flip to account for OpenGL handedness

			m_renderer.updateUniform("MVP", &ubo);

			m_renderer.endScene();
			/* m_pipeline.drawFrame(); */
		}

		m_device.flush();
	}

  private:
	GLFWWindow m_window;
	VulkanInstance m_instance;
	VulkanDevice m_device;
	VulkanRenderer m_renderer;
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;
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
