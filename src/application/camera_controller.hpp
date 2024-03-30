#pragma once

#include "renderer/camera.hpp"
#include "util/memory.hpp"

class CameraController {
  public:
	CameraController(Ref<Camera> cam);
	~CameraController();

  public:
	/**
	 * @brief Processes all input for frame, transforms camera accordingly
	 *
	 * @param dt Timestep
	 */
	void OnUpdate(double dt);

  private:
	void checkForTranslation(double dt);
	void checkForRotation(double dt);

  private:
	Ref<Camera> m_cam;

	float m_translationSpeed = 8.0f;
	float m_rotationSpeed = 15.0f;
	float m_rotationFollow = 0.3f;

	glm::vec2 m_lastMousePos;
	bool m_shouldRotate;
	glm::quat m_target;
};
