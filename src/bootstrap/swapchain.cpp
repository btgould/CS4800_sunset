#include "swapchain.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "device.hpp"
#include "instance.hpp"
#include "util/memory.hpp"
#include "window.hpp"
#include "util/profiler.hpp"

#include "util/log.hpp"
#include "util/constants.hpp"

VulkanSwapChain::VulkanSwapChain(Ref<VulkanInstance> instance, Ref<VulkanDevice> device,
                                 Ref<GLFWWindow> window)
	: m_device(device), m_window(window), m_surface(instance->getSurface()) {
	createSwapChain(m_device, m_window, m_surface);
	createImageViews();
	createOffscreenRenderPass();
	createPostProcessingRenderPass();
	createDepthResources();
	createFramebuffers();
	createOffscreenFrameBufs();
	createSyncObjects();
}

VulkanSwapChain::~VulkanSwapChain() {
	cleanup();

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		// Destroy sync objects
		vkDestroySemaphore(m_device->getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device->getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device->getLogicalDevice(), m_inFlightFences[i], nullptr);
	}

	vkDestroyRenderPass(m_device->getLogicalDevice(), m_offscreenRenderPass, nullptr);
	vkDestroyRenderPass(m_device->getLogicalDevice(), m_postprocessRenderPass, nullptr);
}

void VulkanSwapChain::createSwapChain(const Ref<VulkanDevice> device, const Ref<GLFWWindow> window,
                                      const VkSurfaceKHR surface) {

	// Get swap chain features supported by GPU
	// TODO: when allowing resizes, queried extant becomes out of date before recreation
	SwapChainSupportDetails swapChainSupport = m_device->querySwapChainSupportDetails(surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

	// Make chain 1 longer than min supported
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
	    imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Create swap chain
	VkSwapchainCreateInfoKHR createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Attach queues to swap chain
	QueueFamilyIndices queueFamilyIndices = m_device->getQueueFamilyIndices();
	uint32_t queueFamilyIndicesArr[] = {queueFamilyIndices.graphicsFamily.value(),
	                                    queueFamilyIndices.presentFamily.value()};

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
		// We don't want to explicitly manage queue ownership of an image, let them share
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;     // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// Save handles to relevant objects
	if (vkCreateSwapchainKHR(m_device->getLogicalDevice(), &createInfo, nullptr, &m_swapChain) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_device->getLogicalDevice(), m_swapChain, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device->getLogicalDevice(), m_swapChain, &imageCount,
	                        m_images.data());

	m_imageFormat = surfaceFormat.format;
	m_extent = extent;
}

std::optional<uint32_t> VulkanSwapChain::aquireNextFrame(uint32_t currentFrame) {
	// Wait for previous frame to finish
	vkWaitForFences(m_device->getLogicalDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE,
	                UINT64_MAX);

	// Get image from swap chain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device->getLogicalDevice(), m_swapChain, UINT64_MAX,
	                                        m_imageAvailableSemaphores[currentFrame],
	                                        VK_NULL_HANDLE, &imageIndex);

	switch (result) {
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		// Don't reset fence unless we're going to submit another set of rendering work
		vkResetFences(m_device->getLogicalDevice(), 1, &m_inFlightFences[currentFrame]);
		return imageIndex;
	case VK_ERROR_OUT_OF_DATE_KHR:
		LOG_INFO("Swap chain out of date; recreating");
		recreate();
		return std::nullopt;
	default:
		LOG_ERROR("Unexpected VkResult: {0}", result);
		throw std::runtime_error("failed to acquire swap chain image!");
	}
}

void VulkanSwapChain::submit(VkCommandBuffer cmdBuf, VkPipelineStageFlags* waitStages,
                             uint32_t currentFrame) {
	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Ordered array of semaphores and stages to wait on until semaphore is available
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask =
		waitStages; // implicitly assume to have same size as waitSemaphores, they are paired up
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores =
		&m_renderFinishedSemaphores[currentFrame]; // signal when rendering finishes
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuf; // command buffers to execute

	if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo,
	                  m_inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	m_beenRecreated = false;
}

void VulkanSwapChain::present(uint32_t imageIndex, uint32_t currentFrame) {
	PROFILE_FUNC();
	VkPresentInfoKHR presentInfo {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores =
		&m_renderFinishedSemaphores[currentFrame]; // don't present until done rendering

	VkSwapchainKHR swapChains[] = {m_swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	VkResult result;
	{
		PROFILE_SCOPE("Presenting to queue");
		result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);
	}

	if (m_window->isResized())
		result = VK_SUBOPTIMAL_KHR; // explicitly recreate on resize

	switch (result) {
	case VK_SUCCESS:
		break;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		LOG_INFO("Swap chain out of date; recreating");
		m_window->clearResize();
		recreate();
		break;
	default:
		LOG_ERROR("Unexpected VkResult: {0}", result);
		throw std::runtime_error("failed to present swap chain image!");
	}
}

void VulkanSwapChain::recreate() {
	// don't actually do work until there is something to create
	VkExtent2D framebufferExtant = m_window->getFramebufferSize();
	while (framebufferExtant.width == 0 && framebufferExtant.height == 0) {
		framebufferExtant = m_window->getFramebufferSize();
		glfwWaitEvents();
	}

	// Wait until rendering is finished
	vkDeviceWaitIdle(m_device->getLogicalDevice());

	// Cleanup resources to be recreated
	cleanup();

	// recreate resources
	createSwapChain(m_device, m_window, m_surface);
	createImageViews();
	createDepthResources();
	createFramebuffers();
	createOffscreenFrameBufs();

	m_beenRecreated = true;
}

void VulkanSwapChain::cleanup() {
	vkDestroyImageView(m_device->getLogicalDevice(), m_depthImageView, nullptr);
	vkDestroyImage(m_device->getLogicalDevice(), m_depthImage, nullptr);
	vkFreeMemory(m_device->getLogicalDevice(), m_depthImageMemory, nullptr);

	for (uint32_t i = 0; i < m_framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device->getLogicalDevice(), m_framebuffers[i], nullptr);
		vkDestroyFramebuffer(m_device->getLogicalDevice(), m_offscreenFramebuffers[i].framebuffer,
		                     nullptr);
	}

	for (auto imageView : m_imageViews) {
		vkDestroyImageView(m_device->getLogicalDevice(), imageView, nullptr);
		// TODO: why do I not free the color attachment image memory here? Is it freed when the
		// framebuffer is destroyed? If so, do I need to free the depth attachment memory?
	}

	vkDestroySwapchainKHR(m_device->getLogicalDevice(), m_swapChain, nullptr);
}

VkSurfaceFormatKHR
VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// prefer 4-byte RGBA w/ SRGB color space
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
		    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// otherwise, return first format available
	return availableFormats[0];
}

VkPresentModeKHR
VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	// prefer "triple buffering" (fill queue with newest possible images) when available
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	// otherwise, use basic VSync
	// return VK_PRESENT_MODE_FIFO_KHR;
	// otherwise, present as fast as possible
	// Vulkan, NVIDIA, GLFW, and VSync do not interact well together
	return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                             const Ref<GLFWWindow> window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = window->getFramebufferSize();

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
		                                capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
		                                 capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VulkanSwapChain::createImageViews() {
	m_imageViews.resize(m_images.size());

	for (size_t i = 0; i < m_images.size(); i++) {
		m_imageViews[i] =
			m_device->createImageView(m_images[i], m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

// NOTE: It's possible this should belong to Pipeline (or renderer) instead. However, I may end up
// carrying around multiple copies of highly similar information
void VulkanSwapChain::createOffscreenRenderPass() {
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = m_imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear image before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save rendered image to offscreen tex
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// HACK: postprocessing: I set this to SHADER_READ_ONLY because I am hardcoding a postprocessing
	// step currently. If I don't know about postprocessing, I really want this to be
	// PRESENT_SRC_KHR instead

	VkAttachmentDescription depthAttachment {};
	depthAttachment.format = m_device->findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// create reference to attached image
	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Define subpass to do rendering
	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // this array is index by frag shader outputs
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// Make render subpass depend on image being available
	std::array<VkSubpassDependency, 2> dependencies;

	// Any previous render pass must have finished fragment shading before writing colors
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// We must have finished writing colors before any future render pass can begin fragment shading
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create render pass
	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	// Construct render pass
	if (vkCreateRenderPass(m_device->getLogicalDevice(), &renderPassInfo, nullptr,
	                       &m_offscreenRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanSwapChain::createFramebuffers() {
	m_framebuffers.resize(m_images.size());

	for (uint32_t i = 0; i < m_imageViews.size(); i++) {
		// HACK: does this use the same depth buffer for all images in flight? If so, is that OK?
		std::array<VkImageView, 2> attachments = {m_imageViews[i], m_depthImageView};

		VkFramebufferCreateInfo framebufferInfo {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass =
			m_postprocessRenderPass; // postprocessing pass renders to screen
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_extent.width;
		framebufferInfo.height = m_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device->getLogicalDevice(), &framebufferInfo, nullptr,
		                        &m_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanSwapChain::createSyncObjects() {
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled so that rendering doesn't
	                                                // block forever on first frame

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		if (vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr,
		                      &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
		    vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr,
		                      &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
		    vkCreateFence(m_device->getLogicalDevice(), &fenceInfo, nullptr,
		                  &m_inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects!");
		}
	}
}

void VulkanSwapChain::createDepthResources() {
	VkFormat depthFormat = m_device->findDepthFormat();
	m_device->createImage(m_extent.width, m_extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
	                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
	m_depthImageView =
		m_device->createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanSwapChain::createOffscreenFrameBufs() {
	m_offscreenFramebuffers.resize(m_images.size());
	std::array<VkImageView, 2> imageViews;
	glm::uvec2 postprocessingRes = {getExtent().width, getExtent().height};

	for (uint32_t i = 0; i < m_images.size(); i++) {

		m_offscreenFramebuffers[i].color =
			CreateRef<Texture>(m_device, postprocessingRes, VK_FORMAT_R8G8B8A8_SRGB,
		                       TextureAccessBitFlag::READ_BIT | TextureAccessBitFlag::WRITE_BIT);
		imageViews = {m_offscreenFramebuffers[i].color->getImageView(), m_depthImageView};

		VkFramebufferCreateInfo framebufferInfo {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass =
			m_offscreenRenderPass; // offscreen pass renders to offscreen framebuffer
		framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
		framebufferInfo.pAttachments = imageViews.data();
		framebufferInfo.width = postprocessingRes.x;
		framebufferInfo.height = postprocessingRes.y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device->getLogicalDevice(), &framebufferInfo, nullptr,
		                        &m_offscreenFramebuffers[i].framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanSwapChain::createPostProcessingRenderPass() {
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = m_imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save rendered image to display
	colorAttachment.stencilLoadOp =
		VK_ATTACHMENT_LOAD_OP_DONT_CARE; // We don't use the stencil buffer
	colorAttachment.stencilStoreOp =
		VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't use the stencil buffer
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout =
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Ultimately, we want the color buffer to display to the
	                                     // screen

	VkAttachmentDescription depthAttachment {};
	depthAttachment.format = m_device->findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// create reference to attached image
	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Define subpass to do rendering
	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // thi s array is index by frag shader outputs
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// Make render subpass depend on image being available
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask =
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].dstStageMask =
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask =
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = 0;
	dependencies[1].dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	// Create render pass
	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(m_device->getLogicalDevice(), &renderPassInfo, nullptr,
	                       &m_postprocessRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}
