#include "device.hpp"

#include "instance.hpp"
#include "util/log.hpp"
#include "util/constants.hpp"

#include <stdexcept>
#include <string>
#include <set>
#include <vulkan/vulkan_core.h>

VulkanDevice::VulkanDevice(const VulkanInstance& instance) {
	pickPhysicalDevice(instance);
	createLogicalDevice();
	createCommandPool();
	createCommandBuffers();
}

VulkanDevice::~VulkanDevice() {
	vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
	vkDestroyDevice(m_logicalDevice, nullptr);
}

const VkCommandBuffer VulkanDevice::getFrameCommandBuffer(uint32_t currentFrame) {
	vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
	return m_commandBuffers[currentFrame];
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	// Get types of memory available on device
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	// Find memory type that matches the given typeFilter and supports all the given properties
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
		    (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                VkDeviceMemory& bufferMemory) {

	// Create buffer object
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	// Allocate memory for buffer on GPU
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	// TODO: Hardware limits max simultaneous allocations, need to respect this
	if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	// Associate buffer with memory on GPU
	// 0 here means offset from start of allocated memory
	vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
}

void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	// Create command buffer to do copying
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Record copying to command buffer
	VkBufferCopy copyRegion {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	// Submit command to graphics queue (spec guarantees all graphics queues can copy)
	endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer VulkanDevice::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
}

void VulkanDevice::flush() {
	vkDeviceWaitIdle(m_logicalDevice);
}

void VulkanDevice::pickPhysicalDevice(const VulkanInstance& instance) {
	m_physicalDevice = VK_NULL_HANDLE;

	// select first suitable physical device
	auto surface = instance.getSurface();

	for (const auto& device : instance.getPhysicalDevices()) {
		if (isDeviceSuitable(device, surface)) {
			m_physicalDevice = device;
			m_queueFamilyIndices = findQueueFamilies(device, surface);
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProps);
	LOG_INFO("Selected Physical Device: {0}", deviceProps.deviceName);
	LOG_INFO("\tUsing Vulkan API: {0}.{1}.{2}.{3}", VK_VERSION_MINOR(deviceProps.apiVersion),
	         VK_VERSION_MINOR(deviceProps.apiVersion),
	         VK_API_VERSION_VARIANT(deviceProps.apiVersion),
	         VK_VERSION_PATCH(deviceProps.apiVersion));
	LOG_INFO("\tUsing Driver Version: {0}", deviceProps.driverVersion);
}

void VulkanDevice::createLogicalDevice() {
	float queuePriority = 1.0f;

	// Create graphics queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily.value(),
	                                          m_queueFamilyIndices.presentFamily.value()};

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
	// Create logical device
	VkPhysicalDeviceFeatures deviceFeatures {};

	VkDeviceCreateInfo deviceCreateInfo {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		deviceCreateInfo.enabledLayerCount = requiredValidationLayersSize;
		deviceCreateInfo.ppEnabledLayerNames = requiredValidationLayers;
	} else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// Save handle to graphics queue
	// We use 0 since we just want any queue from the family supporting these actions
	// Therefore, we take the first one.
	vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices.graphicsFamily.value(), 0,
	                 &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices.presentFamily.value(), 0,
	                 &m_presentQueue);
}

void VulkanDevice::createCommandPool() {
	VkCommandPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags =
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // we want to record command buffers
	                                                     // individually, not in groups
	poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void VulkanDevice::createCommandBuffers() {
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // can be submitted for execution, but not
	                                                   // called by other command buffers
	allocInfo.commandBufferCount = m_commandBuffers.size();

	if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_commandBuffers.data()) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(const VkPhysicalDevice device,
                                                   const VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	// Query queue types supported by physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Look for a queue family that supports a graphics queue, save the idx of this family
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport(const VkPhysicalDevice device,
                                                            const VkSurfaceKHR surface) const {
	SwapChainSupportDetails details;

	// Get surface capabilities (i.e. what? TODO)
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Get supported surface image color formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Get supported presentation modes (when images get presented)
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
		                                          details.presentModes.data());
	}

	return details;
}

bool VulkanDevice::isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface) {
	QueueFamilyIndices indices = findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// make sure swap chain can take some type of images and some way to present them
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate =
			!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// NVIDIA causes problems, don't want to deal with that now
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);

	return indices.isComplete() && extensionsSupported &&
	       (strcmp(props.deviceName, "NVIDIA GeForce RTX 3050") != 0);
}

bool VulkanDevice::checkDeviceExtensionSupport(const VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
	                                     availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}
