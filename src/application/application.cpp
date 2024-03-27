#include "application.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include "imgui.h"

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
                                 glm::vec3(300.0f, 200.0f, 75.0f), 1.0f, 10000.0f)),
	  m_camController(CreateRef<CameraController>(m_camera)) {
	if (s_instance) {
		throw std::runtime_error("Tried to create multiple application instances");
	}

	s_instance = this;
}

void Application::run() {
	Model mountain(m_device, "res/model/mountain.obj",
	               TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"),
	               ShaderLibrary::get()->getShader(m_device, "model"));

	Model skybox(m_device, "res/skybox/skybox.obj",
	             TextureLibrary::get()->getTexture(m_device, "res/skybox/skybox.png"),
	             ShaderLibrary::get()->getShader(m_device, "skybox"));
	skybox.getTransform().setScale(glm::vec3(1000.0f, 1000.0f, 1000.0f));
	skybox.getTransform().setTranslation(glm::vec3(300.0f, -500.0f, -300.0f));
	skybox.getTransform().rotateAbout(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(-90.0f));

	Model cloud(m_device, "res/model/cloud.obj",
	            TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"),
	            ShaderLibrary::get()->getShader(m_device, "cloud"));
	cloud.getTransform().setTranslation(glm::vec3(-100.0f, -250.0f, 100.0f));
	cloud.getTransform().setScale(glm::vec3(100.0f, 150.0f, 50.0f));
	Model cloud2(m_device, "res/model/cloud.obj",
	             TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"),
	             ShaderLibrary::get()->getShader(m_device, "cloud"));
	cloud2.getTransform().setTranslation(glm::vec3(-30.0f, 100.0f, 100.0f));
	cloud2.getTransform().setScale(glm::vec3(100.0f, 150.0f, 50.0f));

	CloudSettings cloudSettings;

	LightSource light;
	light.pos = glm::vec3(-630.0f, -500.0f, 267.0f);
	light.color = glm::vec3(0.988f, 0.415f, 0.227f);
	light.ambientStrength = 0.2f;
	light.diffuseStrength = 50.0f;

	while (!m_window->shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		m_window->pollEvents();

		m_renderer->beginScene();

		// Draw models
		m_renderer->draw(mountain);
		m_renderer->draw(skybox);
		m_renderer->draw(cloud);
		m_renderer->draw(cloud2);

		// Draw GUI
		ImGui::ShowDemoWindow();
		ImGui::Begin("Demo");

		ImGui::SeparatorText("Light Settings: ");
		ImGui::DragFloat("Ambient Intensity", &light.ambientStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Diffuse Intensity", &light.diffuseStrength, 1.0f, 0.0f, 100.0f);
		ImGui::DragFloat3("Position", glm::value_ptr(light.pos), 1.0f, -600.0f, 600.0f);
		ImGui::ColorEdit3("Color", glm::value_ptr(light.color));

		ImGui::SeparatorText("Mountain Settings: ");
		glm::vec3 pos = mountain.getTransform().getTranslation();
		ImGui::DragFloat3("Position", glm::value_ptr(pos), 1.0f, -300.0f, 300.0f);
		mountain.getTransform().setTranslation(pos);
		glm::vec3 scale = mountain.getTransform().getScale();
		ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f, 0.1f, 3.0f);
		mountain.getTransform().setScale(scale);

		ImGui::SeparatorText("Cloud Settings: ");
		ImGui::DragFloat("Noise Frequency", &cloudSettings.noiseFreq, 0.1f, 1.0f, 50.0f);
		ImGui::DragFloat("Base intensity", &cloudSettings.baseIntensity, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Opacity", &cloudSettings.opacity, 0.01f, 0.0f, 1.0f);

		ImGui::End();

		m_renderer->endScene();

		// update uniforms
		m_camController->OnUpdate(dt);
		glm::mat4 camVP = m_camera->getVP();
		m_renderer->updateUniform("camVP", &camVP);
		m_renderer->updateUniform("light", &light); // do this in loop b/c >1 framebuffers
		m_renderer->updateUniform("cloudSettings", &cloudSettings);
	}

	m_device->flush();
}

void Application::shutdown() {
	TextureLibrary::get()->cleanup();
	ShaderLibrary::get()->cleanup();
}
