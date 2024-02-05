#include "renderer.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "renderer/texture_lib.hpp"
#include "renderer/vertex_array.hpp"
#include "util/profiler.hpp"
#include "util/constants.hpp"

static void check_vk_result(VkResult err) {
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

VulkanRenderer::VulkanRenderer(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window)
	: m_swapChain(instance, device, window), m_device(device), m_pipeline(device, m_swapChain) {

	// TODO: It would be nice to have a PipelineBuilder class, separate from the Pipeline class for
	// better RAII

	// Configure and create pipeline
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}); // pos
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}); // normal
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}); // color
	m_vertexArray.push({VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 2}); // uv
	m_pipeline.setVertexArray(m_vertexArray);

	m_pushConstantIDs["modelTRS"] =
		m_pipeline.pushPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4));
	m_uniformIDs["camVP"] = m_pipeline.pushUniform(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4));
	m_uniformIDs["cloud"] = m_pipeline.pushUniform(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Cloud));
	m_uniformIDs["cloud2"] = m_pipeline.pushUniform(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Cloud));
	m_uniformIDs["light"] =
		m_pipeline.pushUniform(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(LightSource));

	m_pipeline.pushTexture(TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"));
	m_pipeline.pushTexture(
		TextureLibrary::get()->getTexture(m_device, "res/texture/viking_room.png"));
	m_pipeline.pushTexture(TextureLibrary::get()->getTexture(m_device, "res/skybox/skybox.png"));

	m_pipeline.create();

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void) io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window.getNativeWindow(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance.getNativeInstance();
	init_info.PhysicalDevice = m_device.getPhysicalDevice();
	init_info.Device = m_device.getLogicalDevice();
	init_info.QueueFamily = m_device.getQueueFamilyIndices().graphicsFamily.value();
	init_info.Queue = m_device.getGraphicsQueue();
	init_info.PipelineCache = VK_NULL_HANDLE; // FIXME: this is probably fine?
	init_info.DescriptorPool = m_pipeline.getDescriptorPool();
	init_info.Subpass = 0;
	init_info.MinImageCount = 2; // Just choosing the minimum here for simplicity
	init_info.ImageCount = 2;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr; // Use default allocation mechanism
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, m_swapChain.getRenderPass());
}

VulkanRenderer::~VulkanRenderer() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void VulkanRenderer::beginScene() {
	PROFILE_FUNC();

	// Get image from swap chain
	auto imageIndexOpt = m_swapChain.aquireNextFrame(m_currentFrame);
	if (!imageIndexOpt.has_value()) {
		// Swap chain is recreating, wait until next frame
		return;
	}
	m_imageIndex = imageIndexOpt.value();

	// Prepare to record draw commands
	m_commandBuffer = m_device.getFrameCommandBuffer(m_currentFrame);

	// Start recording
	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;                  // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// Start render pass
	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapChain.getRenderPass();
	renderPassInfo.framebuffer = m_swapChain.getFramebuffer(m_imageIndex);
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_swapChain.getExtent();

	std::array<VkClearValue, 2> clearValues {};
	clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind pipeline
	m_pipeline.bind(m_commandBuffer);

	// New ImGui Frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void VulkanRenderer::draw(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
	vertexBuffer.bind(m_commandBuffer);
	indexBuffer.bind(m_commandBuffer);

	m_pipeline.bindDescriptorSets(m_commandBuffer, m_currentFrame);

	// Draw
	vkCmdDrawIndexed(m_commandBuffer, indexBuffer.size(), 1, 0, 0, 0);
}

void VulkanRenderer::draw(Model& model) {
	model.bind(m_commandBuffer);

	m_pipeline.bindTexture(model.getTexture());
	m_pipeline.bindDescriptorSets(m_commandBuffer, m_currentFrame);

	updatePushConstant("modelTRS", glm::value_ptr(model.getTransform().getTRS()));

	// Draw
	vkCmdDrawIndexed(m_commandBuffer, model.numIndices(), 1, 0, 0, 0);
}

void VulkanRenderer::endScene() {
	// Record ImGui Frame
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(draw_data, m_commandBuffer);

	// End render pass, stop recording
	vkCmdEndRenderPass(m_commandBuffer);
	if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	// submit drawing to GPU queue
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // don't color attachment until image is
	                                                    // available
	m_swapChain.submit(m_commandBuffer, waitStages, m_currentFrame);

	// Present rendered image to screen
	m_swapChain.present(m_imageIndex, m_currentFrame);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::updateUniform(std::string name, void* data) {
	auto uniformID = m_uniformIDs.find(name);

	if (uniformID == m_uniformIDs.end()) {
		// uniform name was not found
		throw std::runtime_error("Unrecognized uniform name");
	}

	m_pipeline.writeUniform(uniformID->second, data, m_currentFrame);
}

void VulkanRenderer::updatePushConstant(const std::string& name, const void* data) {

	auto pushConstantID = m_pushConstantIDs.find(name);

	if (pushConstantID == m_pushConstantIDs.end()) {
		// push constant name was not found
		throw std::runtime_error("Unrecognized push constant name");
	}

	m_pipeline.writePushConstant(m_commandBuffer, pushConstantID->second, data, m_currentFrame);
}
