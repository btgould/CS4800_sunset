#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "device.hpp"
#include "window.hpp"

class VulkanSwapChain {
  public:
	VulkanSwapChain(const VulkanDevice& device, const GLFWWindow& window,
	                const VkSurfaceKHR& surface);
	~VulkanSwapChain();

	VulkanSwapChain(const VulkanSwapChain&) = delete;
	VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

	inline const VkSwapchainKHR& getSwapChain() const { return m_swapChain; }
	inline const VkExtent2D& getSwapChainExtent() const { return m_swapChainExtent; }
	inline const std::vector<VkImageView> getSwapChainImageViews() const {
		return m_swapChainImageViews;
	}
	inline const VkFormat& getSwapChainFormat() const { return m_swapChainImageFormat; }

  private:
	void createSwapChain(const VulkanDevice& device, const GLFWWindow& window,
                                      const VkSurfaceKHR& surface);
	void createImageViews();

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
	const VulkanDevice& m_device;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;
};
