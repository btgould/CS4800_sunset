#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct PipelineDescriptor {
	VkShaderStageFlagBits stage;
	uint32_t size;
};

class Shader {
  public:
	Shader();
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

  private:
	std::string m_vertSource, m_fragSource;

	std::vector<PipelineDescriptor> m_pushConstants, m_uniforms;
};
