#include "swapchain.hpp"
#include "device.hpp"
#include "window.hpp"

#include "util/log.hpp"
#include "util/constants.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VulkanSwapChain::VulkanSwapChain(const VulkanDevice& device, GLFWWindow& window,
                                 const VkSurfaceKHR surface)
	: m_device(device), m_window(window), m_surface(surface) {
	createSwapChain(m_device, m_window, m_surface);
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createSyncObjects();
}

void VulkanSwapChain::createSwapChain(const VulkanDevice& device, const GLFWWindow& window,
                                      const VkSurfaceKHR& surface) {

	// Get swap chain features supported by GPU
	// TODO: when allowing resizes, queried extant becomes out of date before recreation
	SwapChainSupportDetails swapChainSupport = m_device.querySwapChainSupportDetails(surface);

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
	QueueFamilyIndices queueFamilyIndices = m_device.getQueueFamilyIndices();
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
	if (vkCreateSwapchainKHR(m_device.getLogicalDevice(), &createInfo, nullptr, &m_swapChain) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapChain, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapChain, &imageCount, m_images.data());

	m_imageFormat = surfaceFormat.format;
	m_extent = extent;
}

VulkanSwapChain::~VulkanSwapChain() {
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_device.getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device.getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device.getLogicalDevice(), m_inFlightFences[i], nullptr);
	}

	for (auto framebuffer : m_framebuffers) {
		vkDestroyFramebuffer(m_device.getLogicalDevice(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(m_device.getLogicalDevice(), m_renderPass, nullptr);

	for (auto imageView : m_imageViews) {
		vkDestroyImageView(m_device.getLogicalDevice(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device.getLogicalDevice(), m_swapChain, nullptr);
}

std::optional<uint32_t> VulkanSwapChain::aquireNextFrame(uint32_t currentFrame) {
	// Wait for previous frame to finish
	vkWaitForFences(m_device.getLogicalDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE,
	                UINT64_MAX);

	// Get image from swap chain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device.getLogicalDevice(), m_swapChain, UINT64_MAX,
	                                        m_imageAvailableSemaphores[currentFrame],
	                                        VK_NULL_HANDLE, &imageIndex);

	switch (result) {
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		// Don't reset fence unless we're going to submit another set of rendering work
		vkResetFences(m_device.getLogicalDevice(), 1, &m_inFlightFences[currentFrame]);
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

	if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo,
	                  m_inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void VulkanSwapChain::present(uint32_t imageIndex, uint32_t currentFrame) {
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

	VkResult result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);

	if (m_window.isResized())
		result = VK_SUBOPTIMAL_KHR; // explicitly recreate on resize

	switch (result) {
	case VK_SUCCESS:
		break;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		LOG_INFO("Swap chain out of date; recreating");
		m_window.clearResize();
		recreate();
		break;
	default:
		LOG_ERROR("Unexpected VkResult: {0}", result);
		throw std::runtime_error("failed to present swap chain image!");
	}
}

void VulkanSwapChain::recreate() {
	// don't actually do work until there is something to create
	VkExtent2D framebufferExtant = m_window.getFramebufferSize();
	while (framebufferExtant.width == 0 && framebufferExtant.height == 0) {
		framebufferExtant = m_window.getFramebufferSize();
		glfwWaitEvents();
	}

	// Wait until rendering is finished
	vkDeviceWaitIdle(m_device.getLogicalDevice());

	// Cleanup resources to be recreated
	for (size_t i = 0; i < m_framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device.getLogicalDevice(), m_framebuffers[i], nullptr);
	}

	for (size_t i = 0; i < m_imageViews.size(); i++) {
		vkDestroyImageView(m_device.getLogicalDevice(), m_imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device.getLogicalDevice(), m_swapChain, nullptr);

	// recreate resources
	createSwapChain(m_device, m_window, m_surface);
	createImageViews();
	createFramebuffers();
}

VkSurfaceFormatKHR
VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// prefer 4-byte RGBA w/ SRGB color space
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
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
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                             const GLFWWindow& window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = window.getFramebufferSize();

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
		VkImageViewCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_imageFormat;

		// We want to use RGBA components, in that order
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// We want a color image with no mipmapping
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1; // this is used for stereographic stuff

		if (vkCreateImageView(m_device.getLogicalDevice(), &createInfo, nullptr,
		                      &m_imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void VulkanSwapChain::createRenderPass() {
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = m_imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear image before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // save rendered image to display
	colorAttachment.stencilLoadOp =
		VK_ATTACHMENT_LOAD_OP_DONT_CARE; // We don't use the stencil buffer
	colorAttachment.stencilStoreOp =
		VK_ATTACHMENT_STORE_OP_DONT_CARE; // We don't use the stencil buffer
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout =
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Ultimately, we want the color buffer to display to the
	                                     // screen

	// create reference to attached image
	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0; // TODO: this should be abstracted
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Define subpass to do rendering
	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // this array is index by frag shader outputs

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	// Make render pass depend on image being available
	VkSubpassDependency dependency {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0; // 0 here indexes our custom subpasses (of which there is only 1)
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // operation to wait on
	dependency.srcAccessMask = 0;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // operation to prevent until done waiting
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	// Construct render pass
	if (vkCreateRenderPass(m_device.getLogicalDevice(), &renderPassInfo, nullptr, &m_renderPass) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanSwapChain::createFramebuffers() {
	m_framebuffers.resize(m_imageViews.size());

	for (uint32_t i = 0; i < m_imageViews.size(); i++) {
		VkImageView attachments[] = {m_imageViews[i]};

		VkFramebufferCreateInfo framebufferInfo {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_extent.width;
		framebufferInfo.height = m_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device.getLogicalDevice(), &framebufferInfo, nullptr,
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

		if (vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr,
		                      &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
		    vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr,
		                      &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
		    vkCreateFence(m_device.getLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) !=
		        VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects!");
		}
	}
}
