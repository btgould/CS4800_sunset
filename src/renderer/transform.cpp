#include "transform.hpp"
#include <glm/ext/matrix_transform.hpp>

Transform::Transform() {
	m_scale = m_rotation = m_translation = glm::mat4(1.0f);
}

void Transform::setTranslation(const glm::vec3& tr) {
	m_translation = glm::translate(glm::mat4(1.0f), tr);
}

void Transform::translate(const glm::vec3& tr) {
	m_translation = glm::translate(m_translation, tr);
}

void Transform::rotateAbout(const glm::vec3& about, float rad) {
	m_rotation = glm::rotate(m_rotation, rad, about);
}

void Transform::setScale(const glm::vec3& scale) {
	m_scale = glm::scale(glm::mat4(1.0f), scale);
}

void Transform::scale(const glm::vec3& scale) {
	m_scale = glm::scale(m_scale, scale);
}

glm::mat4 Transform::getTRS() {
	m_TRS = m_translation * m_rotation * m_scale;

	return m_TRS;
}
