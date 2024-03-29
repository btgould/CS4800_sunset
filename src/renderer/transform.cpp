#include "transform.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

Transform::Transform() {
	m_translation = glm::vec3(0.0f, 0.0f, 0.0f);
	m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

void Transform::translate(const glm::vec3& tr) {
	m_translation += tr;
}

void Transform::translateRelative(const glm::vec3& tr) {
	m_translation += m_rotation * tr;
}

void Transform::rotateAbout(const glm::vec3& about, float rad) {
	m_rotation = glm::rotate(m_rotation, rad, about) * m_rotation;
}

void Transform::scale(const glm::vec3& scale) {
	m_scale *= scale;
}

glm::mat4 Transform::getTRS() {
	// PERF: I can use a bool to check if I need to recompute this
	m_TRS = glm::translate(glm::mat4(1.0f), m_translation) * glm::mat4_cast(m_rotation) *
	        glm::scale(glm::mat4(1.0f), m_scale);

	return m_TRS;
}

glm::mat4 Transform::getView() {
	// PERF: I can use a bool to check if I need to recompute this
	return glm::mat4_cast(glm::inverse(m_rotation)) *
	       glm::translate(glm::mat4(1.0f), -m_translation);
}
