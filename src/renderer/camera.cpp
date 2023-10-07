#include "camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(float fovY, float aspect, glm::vec3 pos, float nearClip, float farClip)
	: m_fovY(fovY), m_aspect(aspect), m_nearClip(nearClip), m_farClip(farClip), m_pos(pos) {
	m_look = glm::vec3(-1.0f, -1.0f, -1.0f);
	m_proj = glm::perspective(m_fovY, m_aspect, m_nearClip, m_farClip);
	m_proj[1][1] *= -1; // flip to account for OpenGL handedness
}

Camera::~Camera() {}

void Camera::translate(const glm::vec3& tr) {
	m_pos += tr;
}

/* void Camera::rotate(const glm::vec3& axis, const float amount) {} */

void Camera::setTranslation(const glm::vec3& pos) {
	m_pos = pos;
}

/* void Camera::setRotation(const glm::quat& rot) {

}

void Camera::setHeading(const glm::vec3& heading) {
    m_look = heading;
} */
