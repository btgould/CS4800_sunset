#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"

class VulkanInstance {
  public:
	VulkanInstance(GLFWWindow& window);
	~VulkanInstance();

	VulkanInstance(const VulkanInstance&) = delete;
	VulkanInstance& operator=(const VulkanInstance&) = delete;

	inline const VkDevice& getLogicalDevice() const { return m_device.getLogicalDevice(); }
	inline const QueueFamilyIndices getQueueFamilyIndices() const {
		return m_device.getQueueFamilyIndices();
	}
	inline const VkQueue getGraphicsQueue() const { return m_device.getGraphicsQueue(); }
	inline const VkQueue getPresentQueue() const { return m_device.getPresentQueue(); }
	inline const VkSwapchainKHR& getSwapChain() const { return m_swapChain.getSwapChain(); }
	inline const VkExtent2D& getSwapChainExtent() const { return m_swapChain.getSwapChainExtent(); }
	inline const VkFormat& getSwapChainFormat() const { return m_swapChain.getSwapChainFormat(); }
	inline const std::vector<VkImageView> getSwapChainImageViews() const {
		return m_swapChain.getSwapChainImageViews();
	}

  private: // core interface
	void init();
	void createInstance();
	void setupDebugMessenger();
	void createSurface();

  private: // validation / debug
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	                                      const VkAllocationCallbacks* pAllocator,
	                                      VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	                                   const VkAllocationCallbacks* pAllocator);
	static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	                              VkDebugUtilsMessageTypeFlagsEXT messageType,
	                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	                              void* pUserData);

  private:
	GLFWWindow& m_window;
	VulkanDevice m_device;
	VulkanSwapChain m_swapChain;

	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkDebugUtilsMessengerEXT m_debugMessenger;
};
