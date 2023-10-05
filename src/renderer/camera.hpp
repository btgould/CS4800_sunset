#pragma once

#include "glm/glm.hpp"

class Camera {
  public:
	Camera(float fovY, float aspect, glm::vec3 pos = glm::vec3(0), float nearClip = 0.1f,
	       float farClip = 10.0f);
	~Camera();

	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

  public:
	void translate(const glm::vec3& tr);
	void rotate(const glm::vec3& about, const float amount);

	void setTranslation(const glm::vec3& pos);
	void setRotation(const glm::quat& rot);

  private:
	float m_nearClip, m_farClip;

	float m_fovY;

	/* Width / height */
	float m_aspect;

	glm::vec3 m_pos;

	// TODO: model mat should be separate uniform, broken into TRS
	glm::mat4 m_model;
	glm::mat4 m_view;
	glm::mat4 m_proj;
};
