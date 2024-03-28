#include "application.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include "cellular/grid.hpp"
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

	Model dispPlane(m_device, "res/model/plane.obj",
	                TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"),
	                ShaderLibrary::get()->getShader(m_device, "grid"));
	dispPlane.getTransform().setTranslation({9.0f, 0.0f, 0.0f});
	dispPlane.getTransform().scale({3.0f, 3.0f, 3.0f});

	CellGrid grid;
	GridData gridData;

	while (!m_window->shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		// Update window
		m_window->pollEvents();

		// Update sim
		grid.step();
		gridData = grid.getGridData();

		// Render
		m_renderer->beginScene();
		m_renderer->draw(dispPlane);
		m_renderer->endScene();

		// update uniforms
		glm::mat4 camVP = m_camera->getVP();
		m_renderer->updateUniform("camVP", &camVP);
		m_renderer->updateUniform("gridData", &gridData);
	}

	m_device->flush();
}

void Application::shutdown() {
	TextureLibrary::get()->cleanup();
	ShaderLibrary::get()->cleanup();
}
