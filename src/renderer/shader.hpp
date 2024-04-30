#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "bootstrap/device.hpp"

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

struct CloudSettings {
	alignas(4) float noiseFreq = 10.0f;
	alignas(4) float baseIntensity = 0.9f;
	alignas(4) float opacity = 0.8f;
};

struct alignas(128) Atmosphere {
	alignas(16) glm::vec3 center;
	alignas(16) glm::vec3 wavelengths;
	alignas(16) glm::vec3 defractionCoef;
	alignas(4) float time;
	alignas(4) float radius;
	alignas(4) float offsetFactor;
	alignas(4) float densityFalloff;
	alignas(4) float scatteringStrength;
	alignas(4) int numInScatteringPoints;
	alignas(4) int numOpticalDepthPoints;
};

/**
 * @class Shader
 * @brief Describes how pixels of a particular object are colored.
 *
 */
class Shader {
  public:
	Shader(Ref<VulkanDevice> device, const std::string& shaderName);
	~Shader();

  public:
	/**
	 * @brief Gets the push constant available for this shader
	 *
	 * A push constant is a block of variable data used by the shader. This data can be interpreted
	 * as many types, and the chosen type is controlled by the shader source code. Each shader can
	 * have only one push constant.
	 *
	 * Unlike uniforms, push constants can be changed in the middle of a frame.
	 *
	 * @return A struct describing the push constant used by this shader
	 */
	const inline PipelineDescriptor& getPushConstant() const { return m_pushConstant; }

	/**
	 * @brief Gets the set of uniforms available for this shader
	 *
	 * An uniform is a block of variable data used by the shader. Shaders can have many uniforms, of
	 * many different types. Any change to uniform values is not synchronized with the rendering
	 * pipeline. Thus, uniforms should only be changed between frames, never during rendering.
	 *
	 * @return A vector of structs describing the uniforms used by this shader.
	 */
	const inline std::vector<PipelineDescriptor>& getUniforms() const { return m_uniforms; }
	const inline std::array<VkPipelineShaderStageCreateInfo, 2>& getShaderStages() const {
		return m_shaderStages;
	}
	const inline std::string& getName() const { return m_name; }

  private:
	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);

  private:
	Ref<VulkanDevice> m_device;

	PipelineDescriptor m_pushConstant;
	std::vector<PipelineDescriptor> m_uniforms;
	const std::string m_name;

	std::array<VkPipelineShaderStageCreateInfo, 2> m_shaderStages;
	VkShaderModule m_vertShaderModule, m_fragShaderModule;

	static std::unordered_map<std::string, PipelineDescriptor> s_pushConstantMap;
	static std::unordered_map<std::string, std::vector<PipelineDescriptor>> s_uniformMap;
};
