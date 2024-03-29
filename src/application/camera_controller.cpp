#include "camera_controller.hpp"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vulkan/vulkan_core.h>

#include "application/application.hpp"
#include "input.hpp"
#include "util/log.hpp"

CameraController::CameraController(Ref<Camera> cam) : m_cam(cam) {}

CameraController::~CameraController() {}

void CameraController::OnUpdate(double dt) {
	checkForTranslation(dt);
	checkForRotation(dt);
}

void CameraController::checkForTranslation(double dt) {
	glm::vec3 translation(0.0f, 0.0f, 0.0f);

	if (Input::isKeyPressed(GLFW_KEY_W)) {
		translation += glm::vec3(0.0f, 0.0f, -1.0f);
	} else if (Input::isKeyPressed(GLFW_KEY_S)) {
		translation += glm::vec3(0.0f, 0.0f, 1.0f);
	}
	if (Input::isKeyPressed(GLFW_KEY_D)) {
		translation += glm::vec3(1.0f, 0.0f, 0.0f);
	} else if (Input::isKeyPressed(GLFW_KEY_A)) {
		translation += glm::vec3(-1.0f, 0.0f, 0.0f);
	}

	m_cam->translate(translation * m_translationSpeed * (float) dt);
}

void CameraController::checkForRotation(double dt) {
	glm::vec2 newMousePos = Input::getMousePos();
	glm::vec2 displacement =
		m_shouldRotate ? newMousePos - m_lastMousePos
					   : glm::vec2(0.0f, 0.0f); // if this is the first frame we can rotate,
	                                            // zero displacement. Prevents initial "jump"
	displacement *= m_rotationSpeed * dt;
	m_lastMousePos = newMousePos;

	// don't rotate if mouse out of window or not pressed
	VkExtent2D screenSize = Application::get().getWindow()->getFramebufferSize();
	bool mouseOnScreen = (newMousePos.x > 0 && newMousePos.x < screenSize.width &&
	                      newMousePos.y > 0 && newMousePos.y < screenSize.height);
	bool mouseDown = Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_1);
	m_shouldRotate = mouseOnScreen && mouseDown;

	if (!m_shouldRotate) {
		return;
	}

	// Translate screen coordinates to world coords, focus camera
	glm::vec3 mousePos = m_cam->getMousePos(newMousePos, {screenSize.width, screenSize.height});
	glm::quat curr = m_cam->getOrientation();
	glm::quat res = glm::slerp(curr, glm::quatLookAt(mousePos - m_cam->getPos(), m_cam->getUp()),
	                           (float) dt * m_rotationSpeed);
	m_cam->setRotation(res);
}
