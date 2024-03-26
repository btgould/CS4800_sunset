#include "shader_lib.hpp"
#include "util/memory.hpp"

ShaderLibrary* ShaderLibrary::s_instance;

ShaderLibrary::ShaderLibrary() {}

ShaderLibrary* ShaderLibrary::get() {
	if (s_instance == NULL) {
		s_instance = new ShaderLibrary();
	}

	return s_instance;
}

Ref<Shader> ShaderLibrary::getShader(Ref<VulkanDevice> device, const std::string& name) {
	if (m_shaderMap.find(name) != m_shaderMap.end()) {
		return m_shaderMap[name];
	} else {
		auto shader = CreateRef<Shader>(name, device);
		m_shaderMap[name] = shader;
		return shader;
	}
}
void ShaderLibrary::cleanup() {
	m_shaderMap.clear();
}
