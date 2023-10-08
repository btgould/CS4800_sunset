#pragma once

#include "renderer/camera.hpp"

class CameraController {
  public:
	CameraController(Camera& cam);
	~CameraController();

	CameraController(const CameraController&) = delete;
	CameraController& operator=(const CameraController&) = delete;

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
	Camera& m_cam;

	float m_translationSpeed = 8.0f;
	float m_rotationSpeed = 0.1f;

	glm::vec2 m_lastMousePos;
	bool m_mouseOnScreen;
};
