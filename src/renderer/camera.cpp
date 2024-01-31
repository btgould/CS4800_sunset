#include "camera.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "util/log.hpp"

Camera::Camera(float fovY, float aspect, glm::vec3 pos, float nearClip, float farClip)
	: m_fovY(fovY), m_aspect(aspect), m_nearClip(nearClip), m_farClip(farClip), m_pos(pos) {
	lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
	m_proj = glm::perspective(m_fovY, m_aspect, m_nearClip, m_farClip);
	m_proj[1][1] *= -1; // flip to account for OpenGL handedness
}

Camera::~Camera() {}

const glm::mat4 Camera::getVP() {
	// Update data members
	m_view =
		glm::mat4_cast(glm::conjugate(m_orientation)) * glm::translate(glm::mat4(1.0f), -m_pos);

	// Return computed result
	glm::mat4 VP = m_proj * m_view;
	return VP;
}

void Camera::translate(const glm::vec3& tr) {
	m_pos += m_orientation * tr;
}

void Camera::translateAbsolute(const glm::vec3& tr) {
	m_pos += tr;
}

void Camera::setTranslation(const glm::vec3& pos) {
	m_pos = pos;
}

void Camera::lookAt(const glm::vec3& target) {
	m_look = glm::normalize(target - m_pos);

	// prevent needing to change up direction
	if (glm::abs(glm::dot(m_look, m_up)) > m_upThreshold) {
		m_look.z = m_upThreshold * glm::sign(m_look.z);
		m_look = glm::normalize(m_look);
	}

	m_orientation = glm::quatLookAt(m_look, m_up);
}

void Camera::setRotation(const glm::quat& rot) {
	// TODO: keep look vector updated here
	m_orientation = rot;
}
