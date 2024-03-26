#pragma once

#include "bootstrap/device.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

struct PipelineDescriptor {
	VkShaderStageFlagBits stage;
	uint32_t size;
	std::string name;
};

struct LightSource {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(4) float ambientStrength;
	alignas(4) float diffuseStrength;
};

struct Cloud {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 scale;
};

class Shader {
  public:
	Shader(Ref<VulkanDevice> device, const std::string& shaderName);
	~Shader();

  public:
	const inline PipelineDescriptor& getPushConstant() { return m_pushConstant; }
	const inline std::vector<PipelineDescriptor>& getUniforms() { return m_uniforms; }
	const inline std::array<VkPipelineShaderStageCreateInfo, 2> getShaderStages() {
		return m_shaderStages;
	}

  private:
	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);

  private:
	Ref<VulkanDevice> m_device;

	PipelineDescriptor m_pushConstant;
	std::vector<PipelineDescriptor> m_uniforms;

	std::array<VkPipelineShaderStageCreateInfo, 2> m_shaderStages;
	VkShaderModule m_vertShaderModule, m_fragShaderModule;

	static std::unordered_map<std::string, PipelineDescriptor> s_pushConstantMap;
	static std::unordered_map<std::string, std::vector<PipelineDescriptor>> s_uniformMap;
};
