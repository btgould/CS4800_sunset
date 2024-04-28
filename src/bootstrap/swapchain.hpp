#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "device.hpp"
#include "instance.hpp"
#include "window.hpp"

class VulkanSwapChain {
  public:
	VulkanSwapChain(Ref<VulkanInstance> instance, Ref<VulkanDevice> device, Ref<GLFWWindow> window);
	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain&) = delete;

	std::optional<uint32_t> aquireNextFrame(uint32_t currentFrame);
	void submit(VkCommandBuffer cmdBuf, VkPipelineStageFlags* waitStages, uint32_t currentFrame);
	void present(uint32_t imageIndex, uint32_t currentFrame);

	inline const VkRenderPass getOffscreenRenderPass() const { return m_offscreenRenderPass; }
	inline const VkRenderPass getPostProcessRenderPass() const { return m_postprocessRenderPass; }
	/* const std::vector<Ref<Texture>> getOffscreenFramebuffers() const {
	    std::vector<Ref<Texture>> tex;
	    for (const auto& fb : m_offscreenFramebuffers) {
	        tex.push_back(fb.color);
	    }
	    return tex;
	} */
	inline const VkFramebuffer getFramebuffer(uint32_t imageIndex) const {
		return m_framebuffers[imageIndex];
	}
	inline const VkFramebuffer getOffscreenFramebuffer(uint32_t imageIndex) const {
		return m_offscreenFramebuffers[imageIndex];
	}
	inline const VkExtent2D& getExtent() const { return m_extent; }
	inline float getAspectRatio() const { return m_extent.width / (float) m_extent.height; }
	inline bool beenRecreated() const { return m_beenRecreated; }

  private:
	void createSwapChain(const Ref<VulkanDevice> device, const Ref<GLFWWindow> window,
	                     const VkSurfaceKHR surface);
	void createImageViews();
	void createOffscreenRenderPass();
	void createFramebuffers();
	void createSyncObjects();
	void createDepthResources();

	void createOffscreenFrameBufs();
	void createPostProcessingRenderPass();

	void recreate();
	void cleanup();

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
	                            const Ref<GLFWWindow> window);
	inline bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

  private:
	Ref<VulkanDevice> m_device;

	// TODO: this is kinda hacky, but I have to save these handles to recreate
	Ref<GLFWWindow> m_window;
	const VkSurfaceKHR m_surface;
	bool m_beenRecreated = false;

	VkSwapchainKHR m_swapChain;

	VkFormat m_imageFormat;
	VkExtent2D m_extent;

	/* Each render pass instance defines a set of image resources, referred to as attachments, used
	 * during rendering*/
	VkRenderPass m_offscreenRenderPass;
	// NOTE: Could consider using subpasses here to make this more compact, not sure if good
	// performance wise
	VkRenderPass m_postprocessRenderPass;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	std::vector<VkFramebuffer> m_framebuffers;
	std::vector<VkFramebuffer> m_offscreenFramebuffers;

	/* std::vector<Framebuffer> m_offscreenFramebuffers; */

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
};
