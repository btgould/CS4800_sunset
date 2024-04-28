#include "shader.hpp"

#include <fstream>
#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>

// HACK: both of these maps can theoretically be generated at runtime by parsing the shader source
// code. However, I really don't want to do that.
std::unordered_map<std::string, PipelineDescriptor> Shader::s_pushConstantMap = {
	{
		"model",
		{VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), "modelTRS"},
	},
	{
		"cloud",
		{VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), "modelTRS"},
	},
	{
		"skybox",
		{VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), "modelTRS"},
	},
	{
		"atmosphere",
		{},
	},
};

std::unordered_map<std::string, std::vector<PipelineDescriptor>> Shader::s_uniformMap = {
	{"model",
     {
		 {VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), "camVP"},
		 {VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(LightSource), "light"},
	 }},
	{"cloud",
     {
		 {VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), "camVP"},
		 {VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(CloudSettings), "cloudSettings"},
	 }},
	{"skybox",
     {
		 {VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), "camVP"},
	 }},
	{"atmosphere",
     {
		 {VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), "camVP"},
		 {VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Atmosphere), "atmos"},
	 }},
};

Shader::Shader(Ref<VulkanDevice> device, const std::string& shaderName)
	: m_device(device), m_name(shaderName) {
	// Check if we have descriptor data for the given shader
	auto pushConstant = s_pushConstantMap.find(shaderName);
	auto uniforms = s_uniformMap.find(shaderName);
	if (pushConstant == s_pushConstantMap.end() || uniforms == s_uniformMap.end()) {
		throw std::runtime_error("No uniform data provided for shader: " + shaderName +
		                         ". Please hardcode uniform data in bootstrap/shader.cpp");
	}

	m_pushConstant = pushConstant->second;
	m_uniforms = uniforms->second;

	// Init shader
	auto vertShaderCode = readFile("res/shaderc/" + shaderName + ".vert.spv");
	auto fragShaderCode = readFile("res/shaderc/" + shaderName + ".frag.spv");

	m_vertShaderModule = createShaderModule(vertShaderCode);
	m_fragShaderModule = createShaderModule(fragShaderCode);

	// Bind each shader to appropriate pipeline stage
	VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = m_vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = m_fragShaderModule;
	fragShaderStageInfo.pName = "main";

	m_shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
}

Shader::~Shader() {
	vkDestroyShaderModule(m_device->getLogicalDevice(), m_vertShaderModule, nullptr);
	vkDestroyShaderModule(m_device->getLogicalDevice(), m_fragShaderModule, nullptr);
}

std::vector<char> Shader::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to read shader file: " + filename +
		                         ". Have you compiled the shaders?");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	// read entire file contents into buffer
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

VkShaderModule Shader::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device->getLogicalDevice(), &createInfo, nullptr, &shaderModule) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
