#include "application.hpp"

#include <GLFW/glfw3.h>
#include <bits/chrono.h>
#include <stdexcept>
#include <unordered_map>

#include "spdlog/common.h"

#include "tiny_obj_loader.h"

#include "util/log.hpp"
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

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices {};
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex {};

			vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
			              attrib.vertices[3 * index.vertex_index + 1],
			              attrib.vertices[3 * index.vertex_index + 2]};

			vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
			                   1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	VertexBuffer modelVB(m_device, vertices.data(), sizeof(Vertex), vertices.size());
	IndexBuffer modelIB(m_device, indices);

	while (!m_window.shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666;
		m_time = newTime;

		m_window.pollEvents();

		m_renderer.beginScene();
		m_renderer.draw(modelVB, modelIB);

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
