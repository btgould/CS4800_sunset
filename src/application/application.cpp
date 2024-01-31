#include "application.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <stdexcept>

#include "renderer/renderer.hpp"
#include "renderer/texture_lib.hpp"

#include "util/log.hpp"

Application* Application::s_instance = nullptr;

Application::Application()
	: m_window("Vulkan"), m_instance(m_window), m_device(m_instance),
	  m_renderer(m_instance, m_device, m_window),
	  m_camera(glm::radians(45.0f), m_renderer.getAspectRatio(), glm::vec3(300.0f, 200.0f, 75.0f),
               1.0f, 10000.0f),
	  m_camController(m_camera) {
	if (s_instance) {
		throw std::runtime_error("Tried to create multiple application instances");
	}

	s_instance = this;
}

void Application::run() {
	Model model(m_device, "res/model/mountain.obj",
	            TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"));

	Model skybox(m_device, "res/skybox/skybox.obj",
	             TextureLibrary::get()->getTexture(m_device, "res/skybox/skybox.png"));
	skybox.getTransform().setScale(glm::vec3(1000.0f, 1000.0f, 1000.0f));
	skybox.getTransform().setTranslation(glm::vec3(300.0f, -500.0f, -300.0f));
	skybox.getTransform().rotateAbout(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(-90.0f));

	Model cloud(m_device, "res/model/cloud.obj",
	            TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"));
	cloud.getTransform().setTranslation(glm::vec3(-100.0f, -250.0f, 100.0f));
	cloud.getTransform().setScale(glm::vec3(100.0f, 150.0f, 50.0f));
	Cloud u_cloud {cloud.getTransform().getTranslation(), cloud.getTransform().getScale()};
	Model cloud2(m_device, "res/model/cloud.obj",
	             TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"));
	cloud2.getTransform().setTranslation(glm::vec3(-30.0f, 100.0f, 100.0f));
	cloud2.getTransform().setScale(glm::vec3(100.0f, 150.0f, 50.0f));
	Cloud u_cloud2 {cloud2.getTransform().getTranslation(), cloud2.getTransform().getScale()};

	LightSource light;
	light.pos = glm::vec3(-630.0f, -500.0f, 267.0f);
	light.color = glm::vec3(0.988f, 0.415f, 0.227f);
	light.ambientStrength = 0.2f;
	light.diffuseStrength = 50.0f;

	while (!m_window.shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		m_window.pollEvents();

		m_renderer.beginScene();

		// Draw model
		m_renderer.draw(model);
		m_renderer.draw(skybox);
		m_renderer.draw(cloud);
		m_renderer.draw(cloud2);

		m_renderer.endScene();

		// update uniforms
		m_camController.OnUpdate(dt);
		glm::mat4 camVP = m_camera.getVP();
		m_renderer.updateUniform("camVP", &camVP);
		m_renderer.updateUniform("cloud", &u_cloud);
		m_renderer.updateUniform("cloud2", &u_cloud2);
		m_renderer.updateUniform("light", &light); // do this in loop b/c >1 framebuffers
	}

	m_device.flush();
}

void Application::shutdown() {
	TextureLibrary::get()->cleanup();
}
