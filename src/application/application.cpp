#include "application.hpp"

#include <GLFW/glfw3.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glm/glm.hpp>

#include "imgui.h"

#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/shader_lib.hpp"
#include "renderer/texture_lib.hpp"

#include "util/log.hpp"
#include "util/memory.hpp"

Application* Application::s_instance = nullptr;

Application::Application()
	: m_window(CreateRef<GLFWWindow>("Vulkan")), m_instance(CreateRef<VulkanInstance>(m_window)),
	  m_device(CreateRef<VulkanDevice>(m_instance)),
	  m_renderer(CreateRef<VulkanRenderer>(m_instance, m_device, m_window)),
	  m_camera(CreateRef<Camera>(glm::radians(45.0f), m_renderer->getAspectRatio(), glm::vec3(),
                                 1.0f, 10000.0f)),
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
	mountain.getTransform().setTranslation({-300.0f, 10.0f, 250.0f});

	Model cloud(m_device, "res/model/cloud.obj",
	            TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"),
	            ShaderLibrary::get()->getShader(m_device, "cloud"));
	cloud.getTransform().setTranslation(glm::vec3(-400.0f, 110.0f, 500.0f));
	cloud.getTransform().setScale(glm::vec3(100.0f, 50.0f, 150.0f));
	Model cloud2(m_device, "res/model/cloud.obj",
	             TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"),
	             ShaderLibrary::get()->getShader(m_device, "cloud"));
	cloud2.getTransform().setTranslation(glm::vec3(-300.0f, 150.0f, 250.0f));
	cloud2.getTransform().setScale(glm::vec3(100.0f, 50.0f, 150.0f));

	Atmosphere atmos;
	atmos.wavelengths = {700.0f, 530.0f, 440.0f};
	atmos.time = 0;
	atmos.radius = 500.0f;
	atmos.offsetFactor = 0.997f;
	atmos.densityFalloff = 4.0;
	atmos.scatteringStrength = 2.0f;
	atmos.numInScatteringPoints = 10;
	atmos.numOpticalDepthPoints = 10;

	CloudSettings cloudSettings;

	LightSource light;
	light.color = glm::vec3(0.988f, 0.415f, 0.227f);
	light.ambientStrength = 0.1f;
	light.diffuseStrength = 25.0f;

	m_camera->lookAt({-300.0f, 65.0f, 250.0f});

	while (!m_window->shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		m_window->pollEvents();

		m_renderer->beginScene();

		// Draw models
		m_renderer->draw(mountain);
		// m_renderer->draw(skybox);
		m_renderer->draw(cloud);
		m_renderer->draw(cloud2);

		m_renderer->endModelRendering();
		m_renderer->beginUIRendering();

		// Draw GUI
		ImGui::ShowDemoWindow();
		ImGui::Begin("Demo");

		ImGui::SeparatorText("Atmosphere Settings: ");
		ImGui::PushID("Atmosphere");
		ImGui::DragFloat3("Wavelenths", glm::value_ptr(atmos.wavelengths), 1.0f, 100, 1000);
		ImGui::DragFloat("Time of day", &atmos.time, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Radius", &atmos.radius, 10.0f, 100.0f, 1000.0f);
		ImGui::DragFloat("Offset", &atmos.offsetFactor, 0.001f, 0.95f, 1.0f);
		ImGui::DragFloat("Density Falloff", &atmos.densityFalloff, 0.1f, 0.0f, 10.0f);
		ImGui::DragFloat("Scattering Strength", &atmos.scatteringStrength, 0.1f, 0.0f, 10.0f);
		ImGui::DragInt("In Scatter Points", &atmos.numInScatteringPoints, 1.0f, 5, 20);
		ImGui::DragInt("Depth Points", &atmos.numOpticalDepthPoints, 1.0f, 5, 20);
		ImGui::PopID();
		atmos.defractionCoef =
			atmos.scatteringStrength * glm::pow(400.0f / atmos.wavelengths, {4.0f, 4.0f, 4.0f});
		atmos.center = -glm::vec3(0.0f, atmos.radius, 0.0f) * atmos.offsetFactor;
		float theta = atmos.time * glm::pi<float>() / 2.0f;
		light.pos = 1000.0f * glm::vec3(0.0f, glm::cos(theta), -glm::sin(theta)) +
		            mountain.getTransform().getTranslation();

		ImGui::SeparatorText("Light Settings: ");
		ImGui::PushID("Light");
		ImGui::DragFloat("Ambient Intensity", &light.ambientStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Diffuse Intensity", &light.diffuseStrength, 1.0f, 0.0f, 100.0f);
		ImGui::DragFloat3("Position", glm::value_ptr(light.pos), 1.0f, -600.0f, 600.0f);
		ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
		ImGui::PopID();

		ImGui::SeparatorText("Mountain Settings: ");
		ImGui::PushID("Mountain");
		glm::vec3 pos = mountain.getTransform().getTranslation();
		ImGui::DragFloat3("Position", glm::value_ptr(pos), 1.0f, -300.0f, 300.0f);
		mountain.getTransform().setTranslation(pos);
		glm::vec3 scale = mountain.getTransform().getScale();
		ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f, 0.1f, 3.0f);
		mountain.getTransform().setScale(scale);
		ImGui::PopID();

		ImGui::SeparatorText("Cloud Settings: ");
		ImGui::PushID("Cloud");
		ImGui::DragFloat("Noise Frequency", &cloudSettings.noiseFreq, 0.1f, 1.0f, 50.0f);
		ImGui::DragFloat("Base intensity", &cloudSettings.baseIntensity, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Opacity", &cloudSettings.opacity, 0.01f, 0.0f, 1.0f);
		ImGui::PopID();

		ImGui::End();

		m_renderer->endScene();

		// update uniforms
		m_camController->OnUpdate(dt);
		glm::mat4 camVP = m_camera->getVP();
		m_renderer->updateUniform("camVP", &camVP);
		m_renderer->updateUniform("atmos", &atmos);
		m_renderer->updateUniform("light", &light); // do this in loop b/c >1 framebuffers
		m_renderer->updateUniform("cloudSettings", &cloudSettings);
	}

	m_device->flush();
}

void Application::shutdown() {
	TextureLibrary::get()->cleanup();
	ShaderLibrary::get()->cleanup();
}
