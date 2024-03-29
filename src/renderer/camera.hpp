#pragma once

#include "renderer/transform.hpp"
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

	void lookAt(const glm::vec3& target);

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
	inline const glm::vec3& getUp() const { return m_up; }
	inline Transform& getTransform() { return m_transform; }

  private:
	float m_nearClip, m_farClip;
	float m_fovY;
	/* Width / height */
	float m_aspect;

	Transform m_transform;

	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_upThreshold = 0.9f;

	glm::mat4 m_proj;
};
