#pragma once

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
  public:
	Camera(float fovY, float aspect, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	       float nearClip = 0.1f, float farClip = 10.0f);
	~Camera();

  public:
	const glm::mat4 getVP();

	void translate(const glm::vec3& tr);
	void translateAbsolute(const glm::vec3& tr);

	void setTranslation(const glm::vec3& pos);
	void lookAt(const glm::vec3& target);
	void setRotation(const glm::quat& rot);

	/**
	 * @brief Gets a possible position of the mouse pointer in world coordinates.
	 *
	 * Every mouse position corresponds to a ray in world coordinates originating at the camera.
	 * This function returns a point on that ray, with unspecified distance from the camera. If a
	 * specific distance is needed, you will have to compute it externally.
	 *
	 * @param screenCoords The screen space coordinates (in pixels) of the mouse
	 * @param screenSize The size of the screen (in pixels)
	 *
	 * @return 3D world space coordinates of the mouse
	 */
	glm::vec3 getMousePos(const glm::vec2& screenCoords, const glm::vec2& screenSize);

  public:
	inline const glm::vec3& getPos() const { return m_pos; }
	inline const glm::vec3& getLook() const { return m_look; }
	inline const glm::vec3& getUp() const { return m_up; }
	inline const glm::vec3 getRight() const { return glm::cross(m_look, m_up); }
	inline const glm::quat& getOrientation() const { return m_orientation; }

  private:
	float m_nearClip, m_farClip;
	float m_fovY;
	/* Width / height */
	float m_aspect;

	glm::vec3 m_pos;
	glm::quat m_orientation;

	glm::vec3 m_look;
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_upThreshold = 0.9f;

	glm::mat4 m_view;
	glm::mat4 m_proj;
};
