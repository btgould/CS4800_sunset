#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

/* List of indices of queue families supporting a particular queue type, for some physical device.
 * All members are std::optional values. If they have a value, then it will be the index of a queue
 * family supporting that type of queue. Otherwise, no queue family for this devices supports that
 * type of queue.
 */
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	/* Checks if every queue type is supported by some queue family. */
	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanDevice {
  public:
	VulkanDevice(const VkInstance& instance, const VkSurfaceKHR& surface);
	~VulkanDevice();

	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;

	/**
	 * @brief Gets a command buffer from this device. The buffer is guaranteed to be blank and ready
	 * for recording
	 *
	 * @return A VkCommandBuffer in its initial state
	 */
	const VkCommandBuffer getCommandBuffer() const; // FIXME: it's pretty sketchy that this is const

	inline const SwapChainSupportDetails& getSwapChainSupportDetails() const {
		return m_swapChainSupportDetails;
	}
	inline const QueueFamilyIndices& getQueueFamilyIndices() const { return m_queueFamilyIndices; }
	inline const VkDevice& getLogicalDevice() const { return m_logicalDevice; }
	inline const VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	inline const VkQueue getPresentQueue() const { return m_presentQueue; }

  private:
	void pickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface);

	/**
	 * @brief Creates and initializes a logical device
	 *
	 * Creates queue families for each type of operation we need to support, a logical device with
	 * these queues enabled, and saves handles to a queue of each type from the created families.
	 */
	void createLogicalDevice();

	void createCommandPool();
	void createCommandBuffer();

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device,
	                                     const VkSurfaceKHR& surface);
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device,
	                                              const VkSurfaceKHR& surface);
	bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);
	bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

  private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;

	SwapChainSupportDetails m_swapChainSupportDetails;
	QueueFamilyIndices m_queueFamilyIndices;
	VkQueue m_graphicsQueue; // implicitly destroyed with logicalDevice
	VkQueue m_presentQueue;

	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer; // automatically freed with m_commandPool

	const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
