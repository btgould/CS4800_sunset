#include "transform.hpp"
#include <glm/ext/matrix_transform.hpp>

Transform::Transform() {
	m_translation = glm::vec3(0.0f, 0.0f, 0.0f);
	m_rotation = glm::mat4(1.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

void Transform::setTranslation(const glm::vec3& tr) {
	m_translation = tr;
}

void Transform::translate(const glm::vec3& tr) {
	m_translation += tr;
}

void Transform::rotateAbout(const glm::vec3& about, float rad) {
	m_rotation = glm::rotate(m_rotation, rad, about);
}

void Transform::setScale(const glm::vec3& scale) {
	m_scale = scale;
}

void Transform::scale(const glm::vec3& scale) {
	m_scale *= scale;
}

glm::mat4 Transform::getTRS() {
	m_TRS = glm::translate(glm::mat4(1.0f), m_translation) * m_rotation *
	        glm::scale(glm::mat4(1.0f), m_scale);

	return m_TRS;
}
