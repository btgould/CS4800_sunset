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
	 */
	void OnUpdate();

  private:
	void checkForTranslation();
	void checkForRotation();

  private:
	Camera& m_cam;

	float m_translationSpeed = 5.0f;
	float m_rotationSpeed = 0.01f;

	glm::vec2 m_lastMousePos;
	bool m_mouseOnScreen;
};
