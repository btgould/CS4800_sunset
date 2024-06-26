#include "renderer.hpp"

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <iterator>
#include <vulkan/vulkan_core.h>

#include "bootstrap/pipeline.hpp"
#include "renderer/shader_lib.hpp"
#include "renderer/texture_lib.hpp"
#include "util/memory.hpp"
#include "util/profiler.hpp"
#include "util/constants.hpp"
#include "util/log.hpp"

static void check_vk_result(VkResult err) {
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

VulkanRenderer::VulkanRenderer(Ref<VulkanInstance> instance, Ref<VulkanDevice> device,
                               Ref<GLFWWindow> window)
	: m_swapChain(CreateRef<VulkanSwapChain>(instance, device, window)), m_device(device),
	  m_pipelineBuilder(device, m_swapChain),
	  m_defaultVA({{VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}, // pos
                   {VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}, // normal
                   {VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 3}, // color
                   {VertexAtrributeType::VERTEX_ATTRIB_TYPE_F32, 2}} // uv
                  ),
	  m_textures({
		  TextureLibrary::get()->getTexture(m_device, "res/texture/mountain.png"),
		  TextureLibrary::get()->getTexture(m_device, "res/texture/viking_room.png"),
		  TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"),
		  TextureLibrary::get()->getTexture(m_device, "res/skybox/skybox.png"),
	  }) {
	auto shader = ShaderLibrary::get()->getShader(m_device, "model");
	m_pipelines.push_back(m_pipelineBuilder.buildPipeline(m_defaultVA, shader, m_textures));
	m_activePipeline = m_pipelines[0];

	// Setup postprocessing
	m_postprocessPipeline = m_pipelineBuilder.buildPipeline(
		VertexArray(), ShaderLibrary::get()->getShader(m_device, "atmosphere"),
		{TextureLibrary::get()->getTexture(m_device, "res/texture/default.png")}, true);
	m_postprocessPipeline->bindTexture(
		TextureLibrary::get()->getTexture(m_device, "res/texture/default.png"));

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void) io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window->getNativeWindow(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance->getNativeInstance();
	init_info.PhysicalDevice = m_device->getPhysicalDevice();
	init_info.Device = m_device->getLogicalDevice();
	init_info.QueueFamily = m_device->getQueueFamilyIndices().graphicsFamily.value();
	init_info.Queue = m_device->getGraphicsQueue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_postprocessPipeline->getDescriptorPool();
	init_info.Subpass = 0;
	init_info.MinImageCount = 2; // Just choosing the minimum here for simplicity
	init_info.ImageCount = 2;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr; // Use default allocation mechanism
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, m_swapChain->getPostProcessRenderPass());
}

VulkanRenderer::~VulkanRenderer() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void VulkanRenderer::beginScene() {
	PROFILE_FUNC();

	// Get image from swap chain
	auto imageIndexOpt = m_swapChain->aquireNextFrame(m_currentFrame);
	if (!imageIndexOpt.has_value()) {
		// Swap chain is recreating, wait until next frame
		return;
	}
	m_imageIndex = imageIndexOpt.value();

	// Prepare to record draw commands
	m_commandBuffer = m_device->getFrameCommandBuffer(m_currentFrame);

	// Start recording
	// PERF: This seems like I could record most of the commands once, leave small parts separate
	// (i.e. uploading push constants), and reduce the time spent recording every frame
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
	renderPassInfo.renderPass = m_swapChain->getOffscreenRenderPass();
	renderPassInfo.framebuffer = m_swapChain->getOffscreenFramebuffer(m_imageIndex);
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_swapChain->getExtent();

	std::array<VkClearValue, 2> clearValues {};
	clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind pipeline
	m_pipelines[0]->bind(m_commandBuffer);
}

void VulkanRenderer::draw(Model& model) {
	PROFILE_FUNC();
	if (!m_activePipeline->canRender(model)) {
		findOrBuildPipeline(model);
	}

	model.bind(m_commandBuffer);

	m_activePipeline->bindTexture(model.getTexture());
	m_activePipeline->bindDescriptorSets(m_commandBuffer, m_currentFrame);

	updatePushConstant("modelTRS", glm::value_ptr(model.getTransform().getTRS()));

	// Draw
	vkCmdDrawIndexed(m_commandBuffer, model.numIndices(), 1, 0, 0, 0);
}

void VulkanRenderer::endModelRendering() {
	// End main render pass
	vkCmdEndRenderPass(m_commandBuffer);

	// postprocessing
	{
		PROFILE_SCOPE("postprocessing");
		VkRenderPassBeginInfo renderPassInfo {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapChain->getPostProcessRenderPass();
		renderPassInfo.framebuffer = m_swapChain->getFramebuffer(m_imageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = m_swapChain->getExtent();

		std::array<VkClearValue, 1> clearValues {};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		m_postprocessPipeline->bind(m_commandBuffer);
		/* m_postprocessPipeline->bindTexture(m_swapChain->getOffscreenFramebuffer(0).color); */
		m_postprocessPipeline->bindDescriptorSets(m_commandBuffer, m_currentFrame);
		vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);
	}
}

void VulkanRenderer::beginUIRendering() {
	// New ImGui Frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void VulkanRenderer::endScene() {
	PROFILE_FUNC();
	// Record ImGui Frame
	m_activePipeline = m_postprocessPipeline;
	m_activePipeline->bind(m_commandBuffer);

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(draw_data, m_commandBuffer);

	// ImGui::RenderPlatformWindowsDefault();
	// ImGui::UpdatePlatformWindows();

	vkCmdEndRenderPass(m_commandBuffer);

	// Finish recording commands, submit drawing to GPU queue
	if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // don't color attachment until image is
	                                                    // available
	m_swapChain->submit(m_commandBuffer, waitStages, m_currentFrame);

	// Present rendered image to screen
	m_swapChain->present(m_imageIndex, m_currentFrame);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::updateUniform(std::string name, void* data) {
	for (auto pipeline : m_pipelines) {
		pipeline->writeUniform(name, data, m_currentFrame);
	}

	m_postprocessPipeline->writeUniform(name, data, m_currentFrame);
}

void VulkanRenderer::updatePushConstant(const std::string& name, const void* data) {
	for (auto pipeline : m_pipelines) {
		pipeline->writePushConstant(m_commandBuffer, name, data, m_currentFrame);
	}
}

void VulkanRenderer::findOrBuildPipeline(const Model& model) {
	bool compatiblePipeline = false;
	for (const auto pipeline : m_pipelines) {
		if (pipeline->canRender(model)) {
			m_activePipeline = pipeline;
			pipeline->bind(m_commandBuffer);
			compatiblePipeline = true;
			break;
		}
	}

	if (!compatiblePipeline) {
		LOG_WARN("Pipeline-model mismatch!");
		LOG_INFO("Constructing new pipeline");
		auto pipeline = m_pipelineBuilder.buildPipeline(m_defaultVA, model.getShader(), m_textures);
		m_activePipeline = pipeline;
		pipeline->bind(m_commandBuffer);
		m_pipelines.push_back(pipeline);
	}
}
