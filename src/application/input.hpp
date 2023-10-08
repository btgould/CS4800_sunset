#pragma once

#include <glm/ext/vector_float2.hpp>

class Input {
  public:
	static bool isKeyPressed(int key);
	static bool isMouseButtonPressed(int button);
	static glm::vec2 getMousePos();

  private:
	Input();
	~Input();
};
