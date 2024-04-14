#include "application.hpp"

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include "application/input.hpp"
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
}

glm::vec2 getHexCoord(const glm::vec2& pos) {
	const glm::mat2 cart2hex =
		glm::mat2({{2.0f / glm::sqrt(3.0f), 0.0f}, {1.0f / glm::sqrt(3.0f), 1.0f}});
	const glm::mat2 hex2cart = glm::mat2({{glm::sqrt(3.0f) / 2.0f, 0.0f}, {-0.5f, 1.0f}});
	glm::vec2 q = pos * cart2hex;

	glm::vec2 qInt = glm::floor(q);
	glm::vec2 qFrac = glm::fract(q);
	glm::vec2 qFracT = {qFrac.y, qFrac.x};
	float v = glm::mod(qInt.x + qInt.y, 3.0f);

	float ca = glm::step(1.0f, v);
	float cb = glm::step(2.0f, v);
	glm::vec2 ma = step(qFrac, qFracT);

	glm::vec2 hexCoord = (qInt + ca - cb * ma) * hex2cart;
	hexCoord.x = glm::floor(hexCoord.x / glm::sqrt(3.0f));
	hexCoord.y /= 1.5f;
	return hexCoord;
}

void Application::run() {
	Model dispPlane(m_device, "res/model/plane.obj",
	                TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"),
	                ShaderLibrary::get()->getShader(m_device, "grid"));
	dispPlane.getTransform().setTranslation({1.5f, 0.0f, -9.0f});
	dispPlane.getTransform().scale({3.0f, 3.0f, 3.0f});

	CellGrid grid;
	GridData gridData;
	CellType penType = CellType::CELL_TYPE_EMPTY;

	while (!m_window->shouldClose()) {
		double newTime = glfwGetTime();
		double dt = (newTime - m_time) / 0.0166666; // convert dt to units of frames, at 60fps
		m_time = newTime;

		// Update window
		m_window->pollEvents();

		// Map mouse position onto grid
		auto screenSize = m_renderer->getExtent();
		auto mousePos =
			m_camera->getMousePos(Input::getMousePos(), {screenSize.width, screenSize.height});
		mousePos *= (dispPlane.getTransform().getTranslation().z / mousePos.z);

		auto corner = dispPlane.getTransform().getTRS() * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
		mousePos.x = (mousePos.x - corner.x) / (2 * dispPlane.getTransform().getScale().x);
		mousePos.y = (corner.y - mousePos.y) / (2 * dispPlane.getTransform().getScale().y);

		mousePos *= GRID_COUNT + 1;
		mousePos.x -= 0.5f;
		mousePos.y -= 1.0f / glm::sqrt(3.0f);
		mousePos *= glm::sqrt(3.0f);
		glm::vec2 gridCoord = getHexCoord(mousePos);

		// Write data to grid
		if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_1) && mousePos.x >= 0 &&
		    mousePos.x < GRID_COUNT && mousePos.y >= 0 && mousePos.y < GRID_COUNT) {
			grid.write(penType, mousePos.y, mousePos.x);
		}

		// Update sim
		grid.step(dt);
		gridData = grid.getGridData();

		// Render
		m_renderer->beginScene();
		m_renderer->draw(dispPlane);

		ImGui::Begin("Options");

		if (ImGui::Button("Clear")) {
			grid.clear();
		}

		ImGui::SeparatorText("Pen");
		if (ImGui::RadioButton("Empty", penType == CellType::CELL_TYPE_EMPTY)) {
			penType = CellType::CELL_TYPE_EMPTY;
		}
		if (ImGui::RadioButton("Solid", penType == CellType::CELL_TYPE_SOLID)) {
			penType = CellType::CELL_TYPE_SOLID;
		}
		if (ImGui::RadioButton("Fluid", penType == CellType::CELL_TYPE_FLUID_LEFT)) {
			penType = CellType::CELL_TYPE_FLUID_LEFT;
		}
		if (ImGui::RadioButton("Sand", penType == CellType::CELL_TYPE_SAND)) {
			penType = CellType::CELL_TYPE_SAND;
		}
		if (ImGui::RadioButton("Fungi", penType == CellType::CELL_TYPE_FUNGI)) {
			penType = CellType::CELL_TYPE_FUNGI;
		}

		ImGui::End();

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
