#include "camera.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>

#include "util/log.hpp"

Camera::Camera(float fovY, float aspect, glm::vec3 pos, float nearClip, float farClip)
	: m_fovY(fovY), m_aspect(aspect), m_nearClip(nearClip), m_farClip(farClip) {
	lookAt(glm::vec3(0.0f, 0.0f, -1.0f));
	m_proj = glm::perspective(m_fovY, m_aspect, m_nearClip, m_farClip);
	m_proj[1][1] *= -1; // flip to account for OpenGL inverting y-axis
}

Camera::~Camera() {}

const glm::mat4 Camera::getVP() {
	glm::mat4 VP = m_proj * m_transform.getView();
	return VP;
}

void Camera::lookAt(const glm::vec3& target) {
	glm::vec3 look = glm::normalize(target - m_transform.getTranslation());

	// prevent needing to change up direction
	if (glm::abs(glm::dot(look, m_up)) > m_upThreshold) {
		look.y = m_upThreshold * glm::sign(look.y); // HACK: this is coordinate system dependent
		look = glm::normalize(look);
	}

	m_transform.setRotation(glm::quatLookAt(look, m_up));
}

glm::vec3 Camera::getMousePos(const glm::vec2& screenCoords, const glm::vec2& screenSize) {
	// Screen -> normalized device -> clip space (all in one group for efficiency)
	glm::vec4 coords = glm::vec4(screenCoords.x, screenCoords.y, 0.0f, 1.0f);
	coords.x = (2.0f * coords.x) / screenSize.x - 1.0f;
	coords.y = (2.0f * coords.y) / screenSize.y - 1.0f;

	// clip space -> world coords
	coords = glm::inverse(getVP()) * coords;
	coords /= coords.w;

	// I could just use glm::unProject, but I wrote it out here for readability
	return glm::vec3(coords);
}
