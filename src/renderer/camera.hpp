#pragma once

#include "glm/glm.hpp"
#include <glm/ext/matrix_transform.hpp>

class Camera {
  public:
	Camera(float fovY, float aspect, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	       float nearClip = 0.1f, float farClip = 10.0f);
	~Camera();

  public:
	inline const glm::mat4 getVP() {
		// Update data members
		m_view = glm::lookAt(m_pos, m_pos + m_look, glm::vec3(0.0f, 0.0f, 1.0f));

		// Return computed result
		return m_proj * m_view;
	}

	void translate(const glm::vec3& tr);
	/* void rotate(const glm::vec3& axis, const float amount); */

	void setTranslation(const glm::vec3& pos);
	/* void setRotation(const glm::quat& rot);
	void setHeading(const glm::vec3& heading); */

  private:
	float m_nearClip, m_farClip;
	float m_fovY;
	/* Width / height */
	float m_aspect;

	glm::vec3 m_pos;
	glm::vec3 m_look;

	glm::mat4 m_view;
	glm::mat4 m_proj;
};
