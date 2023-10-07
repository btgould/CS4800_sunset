#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "device.hpp"
#include "instance.hpp"
#include "window.hpp"

class VulkanSwapChain {
  public:
	VulkanSwapChain(VulkanInstance& instance, VulkanDevice& device, GLFWWindow& window);
	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain&) = delete;
	VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

	std::optional<uint32_t> aquireNextFrame(uint32_t currentFrame);
	void submit(VkCommandBuffer cmdBuf, VkPipelineStageFlags* waitStages, uint32_t currentFrame);
	void present(uint32_t imageIndex, uint32_t currentFrame);

	inline const VkSwapchainKHR& getSwapChain() const { return m_swapChain; }
	inline const VkRenderPass getRenderPass() const { return m_renderPass; }
	inline const std::vector<VkImageView> getImageViews() const { return m_imageViews; }
	inline const VkFormat& getImageFormat() const { return m_imageFormat; }
	inline const VkFramebuffer getFramebuffer(uint32_t imageIndex) const {
		return m_framebuffers[imageIndex];
	}
	inline const VkExtent2D& getExtent() const { return m_extent; }
	inline float getAspectRatio() const { return m_extent.width / (float) m_extent.height; }

  private:
	void createSwapChain(const VulkanDevice& device, const GLFWWindow& window,
	                     const VkSurfaceKHR& surface);
	void createImageViews();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();

	void recreate();

	VkSurfaceFormatKHR
	chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR
	chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	/**
	 * @brief Calculates the extent of the surfaces to render to (in pixels)
	 *
	 * This cannot just use screen coordinates directly, since it has to account for monitor DPI
	 *
	 * @param capabilities Capabilities of the render surface
	 * @return Extent (in pixels) of the render region
	 */
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
	                            const GLFWWindow& window);

  private:
	VulkanDevice& m_device;

	// TODO: this is kinda hacky, but I have to save these handles to recreate
	GLFWWindow& m_window;
	const VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	VkFormat m_imageFormat;
	VkExtent2D m_extent;

	/* Each render pass instance defines a set of image resources, referred to as attachments, used
	 * during rendering*/
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_framebuffers;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
};
