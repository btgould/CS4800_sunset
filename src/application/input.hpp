#pragma once

#include <utility>

class Input {
  public:
	inline static Input get() {
		if (s_initialized) {
			return s_instance;
		} else {
			s_instance = Input();
			s_initialized = true;
			return s_instance;
		}
	}

  public:
	bool isKeyPressed(int key);
	bool isMouseButtonPressed(int button);
	std::pair<float, float> getMousePos();

  private:
	Input();
	~Input();

  private:
	static bool s_initialized;
	static Input s_instance;
};
