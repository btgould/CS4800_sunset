#include "application.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <unordered_map>

#include "renderer/texture_lib.hpp"
#include "spdlog/common.h"

#include "util/log.hpp"
#include "util/memory.hpp"
#include "util/profiler.hpp"
#include "input.hpp"

Application* Application::s_instance = nullptr;

Application::Application()
	: m_window("Vulkan"), m_instance(m_window), m_device(m_instance),
	  m_renderer(m_instance, m_device, m_window),
	  m_camera(glm::radians(45.0f), m_renderer.getAspectRatio(), glm::vec3(300.0f, 300.0f, 300.0f),
               1.0f, 1000.0f),
	  m_camController(m_camera) {
	if (s_instance) {
		throw std::runtime_error("Tried to create multiple application instances");
	}

	s_instance = this;

	m_modelTranslation = glm::mat4(1.0f);
	m_modelRotation = glm::mat4(1.0f);
	m_modelScale = glm::mat4(1.0f);
}

void Application::run() {
	const std::string modelPath = "res/model/mountain.obj";
	const std::string texPath = "res/texture/mountain.png";

	Ref<Texture> modelTex = TextureLibrary::get()->getTexture(m_device, texPath);
	Model model(m_device, modelPath, modelTex);
	Model room(m_device, "res/model/viking_room.obj",
	           TextureLibrary::get()->getTexture(m_device, "res/texture/viking_room.png"));

	while (!m_window.shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		m_window.pollEvents();

		m_renderer.beginScene();
		m_renderer.draw(model);
		m_renderer.draw(room);

		// update uniforms
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();

		m_modelRotation = glm::rotate(m_modelRotation, (float) dt * glm::radians(1.0f),
		                              glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 modelTRS = m_modelScale * m_modelRotation * m_modelTranslation;
		m_renderer.updateUniform("modelTRS", &modelTRS);

		m_camController.OnUpdate(dt);
		glm::mat4 camVP = m_camera.getVP();
		m_renderer.updateUniform("camVP", &camVP);

		m_renderer.endScene();
	}

	m_device.flush();
}

void Application::shutdown() {
	TextureLibrary::get()->cleanup();
}
