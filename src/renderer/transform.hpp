#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
  public:
	Transform();
	~Transform() = default;

  public:
	inline const glm::vec3& getTranslation() const { return m_translation; }
	inline void setTranslation(const glm::vec3& tr) { m_translation = tr; }
	/**
	 * @brief Translates the object by the given vector
	 *
	 * @param tr The vector to translate the object by, in world coordinates
	 */
	void translate(const glm::vec3& tr);
	/**
	 * @brief Translates the object by the given vector *relative to its orientation*
	 *
	 * @param tr The vector to translate the object by, in local coordinates
	 */
	void translateRelative(const glm::vec3& tr);

	inline const glm::quat& getRotation() const { return m_rotation; }
	inline void setRotation(const glm::quat& rot) { m_rotation = rot; }
	void rotateAbout(const glm::vec3& about, float rad);

	inline const glm::vec3& getScale() const { return m_scale; }
	inline void setScale(const glm::vec3& scale) { m_scale = scale; }
	void scale(const glm::vec3& scale);

	glm::mat4 getTRS();
	glm::mat4 getView();

  private:
	glm::vec3 m_translation;
	glm::quat m_rotation;
	glm::vec3 m_scale;

	glm::mat4 m_TRS;
};
