#pragma once

#include "instance.hpp"
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
	/**
	 * @brief Creates a device to run the given instance
	 *
	 * @param instance The instance to run on the device
	 */
	VulkanDevice(const VulkanInstance& instance);

	/**
	 * @brief Cleans up all resources used by this device
	 */
	~VulkanDevice();

	VulkanDevice(const VulkanDevice&) = delete;
	VulkanDevice& operator=(const VulkanDevice&) = delete;

	/**
	 * @brief Gets a command buffer from this device. The buffer is guaranteed to be blank and ready
	 * for recording, and to come from a command pool supporting graphics operations.
	 *
	 * @return A VkCommandBuffer in its initial state
	 */
	const VkCommandBuffer getFrameCommandBuffer(uint32_t currentFrame);

	/**
	 * @brief Gets the index of a memory type on the GPU matching the given type filter and
	 * supporting all of the desired properties
	 *
	 * @param typeFilter bitflag representing the allowable memory types
	 * @param properties bitflag representing the required memory properties
	 * @return The index of a memory type matching both the type filter and properties
	 */
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	/**
	 * @brief Creates a buffer
	 *
	 * @param size The size in bytes of the buffer to create
	 * @param usage Bitflag indicating the intended use case of this buffer (affects cache friendly
	 * allocation procedures)
	 * @param properties Bitflag indicating the required type of memory to allocate for this buffer
	 * @param buffer Handle to the buffer object to create
	 * @param bufferMemory Handle to the memory allocated for this buffer
	 */
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	                  VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	/**
	 * @brief Copies data from srcBuffer to dstBuffer
	 *
	 * @param srcBuffer The source of the data to copy
	 * @param dstBuffer The location to copy to
	 * @param size The amount of data to copy
	 */
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	/**
	 * @brief Creates an image object on the GPU
	 *
	 * @param width Width of the image in texels
	 * @param height Height of the image in texels
	 * @param format Describes the data types used to store each texel
	 * @param tiling Describes how texels should be laid out in memory
	 * @param usage Describes what the image will be used for
	 * @param properties Required properties for the allocated memory to support
	 * @param image Image object to create the image to
	 * @param imageMemory GPU memory to store the image in
	 */
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
	                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
	                 VkDeviceMemory& imageMemory);

	/**
	 * @brief Changes the layout of the given image from oldLayout to newLayout
	 *
	 * @param image The image to transition
	 * @param format The data types used to represent each texel of the image
	 * @param oldLayout The previous ordering of image texels in memory
	 * @param newLayout The desired ordering of image texels in memory
	 */
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
	                           VkImageLayout newLayout);

	/**
	 * @brief Copies a buffer object into an image object
	 *
	 * @param buffer The buffer to copy data from
	 * @param image The image to copy data into
	 * @param width The width of the image to copy
	 * @param height The height of the image to copy
	 */
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	/**
	 * @brief Wait until all pending commands on this device have been executed
	 */
	void flush();

	inline const SwapChainSupportDetails
	querySwapChainSupportDetails(const VkSurfaceKHR surface) const {
		return querySwapChainSupport(m_physicalDevice, surface);
	}
	inline const QueueFamilyIndices& getQueueFamilyIndices() const { return m_queueFamilyIndices; }
	inline const VkDevice getLogicalDevice() const { return m_logicalDevice; }
	inline const VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	inline const VkQueue getPresentQueue() const { return m_presentQueue; }

  private:
	void pickPhysicalDevice(const VulkanInstance& instance);

	/**
	 * @brief Creates and initializes a logical device
	 *
	 * Creates queue families for each type of operation we need to support, a logical device with
	 * these queues enabled, and saves handles to a queue of each type from the created families.
	 */
	void createLogicalDevice();

	void createCommandPool();
	void createCommandBuffers();
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface);
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device,
	                                              const VkSurfaceKHR surface) const;
	bool checkDeviceExtensionSupport(const VkPhysicalDevice device);
	bool isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface);

  private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;

	QueueFamilyIndices m_queueFamilyIndices;
	VkQueue m_graphicsQueue; // implicitly destroyed with logicalDevice
	VkQueue m_presentQueue;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers; // automatically freed with m_commandPool

	const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
