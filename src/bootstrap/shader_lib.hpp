#pragma once

#include <map>
#include <string>

#include "bootstrap/shader.hpp"

class ShaderLibrary {

  private:
	static ShaderLibrary* s_instance;

	ShaderLibrary();
	~ShaderLibrary() = default;
	ShaderLibrary(const ShaderLibrary&) = delete;

  public:
	static ShaderLibrary* get();
	Ref<Shader> getShader(Ref<VulkanDevice> device, const std::string& name);
	void cleanup();

  private:
	std::map<std::string, Ref<Shader>> m_shaderMap;
};
