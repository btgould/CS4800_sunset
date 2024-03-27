#include "application.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include "imgui.h"

#include "renderer/model.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader_lib.hpp"
#include "renderer/texture_lib.hpp"

#include "util/log.hpp"
#include "util/memory.hpp"

Application* Application::s_instance = nullptr;

Application::Application()
	: m_window(CreateRef<GLFWWindow>("Vulkan")), m_instance(CreateRef<VulkanInstance>(m_window)),
	  m_device(CreateRef<VulkanDevice>(m_instance)),
	  m_renderer(CreateRef<VulkanRenderer>(m_instance, m_device, m_window)),
	  m_camera(CreateRef<Camera>(glm::radians(45.0f), m_renderer->getAspectRatio(),
                                 glm::vec3(0.0f, 0.0f, 0.0f), 0.1f, 100.0f)) {
	if (s_instance) {
		throw std::runtime_error("Tried to create multiple application instances");
	}

	s_instance = this;

	// m_camera->lookAt(glm::vec3());
}

void Application::run() {
	m_camera->lookAt({1.0f, 0.0f, 0.0f});

	Model grid(m_device, "res/model/plane.obj",
	           TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"),
	           ShaderLibrary::get()->getShader(m_device, "grid"));
	grid.getTransform().setTranslation({9.0f, 0.0f, 0.0f});
	grid.getTransform().scale({3.0f, 3.0f, 3.0f});

	while (!m_window->shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		m_window->pollEvents();

		m_renderer->beginScene();

		// Draw grid
		m_renderer->draw(grid);

		m_renderer->endScene();

		// update uniforms
		glm::mat4 camVP = m_camera->getVP();
		m_renderer->updateUniform("camVP", &camVP);
	}

	m_device->flush();
}

void Application::shutdown() {
	TextureLibrary::get()->cleanup();
	ShaderLibrary::get()->cleanup();
}
