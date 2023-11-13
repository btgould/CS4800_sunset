#pragma once

#include <glm/glm.hpp>

class Transform {
  public:
	Transform();
	~Transform() = default;

	Transform(const Transform&) = delete;
	Transform& operator=(const Transform&) = delete;

  public:
	void setTranslation(const glm::vec3& tr);
	void translate(const glm::vec3& tr);

	void rotateAbout(const glm::vec3& about, float rad);

	void setScale(const glm::vec3& scale);
	void scale(const glm::vec3& scale);

	glm::mat4 getTRS();

  private:
	glm::mat4 m_translation;
	glm::mat4 m_rotation;
	glm::mat4 m_scale;

	glm::mat4 m_TRS;
};
