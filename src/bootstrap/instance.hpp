#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "window.hpp"

class VulkanInstance {
  public:
	VulkanInstance(GLFWWindow& window);
	~VulkanInstance();

	VulkanInstance(const VulkanInstance&) = delete;
	VulkanInstance& operator=(const VulkanInstance&) = delete;

	/**
	 * @brief Gets a list of physical devices available for use with this instance
	 */
	std::vector<VkPhysicalDevice> getPhysicalDevices() const;

	/* inline VulkanDevice& getDevice() { return m_device; } */
	inline VkSurfaceKHR getSurface() const { return m_surface; }
	// HACK: this exists only to satisfy ImGui
	inline VkInstance getNativeInstance() const { return m_instance; }

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

	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkDebugUtilsMessengerEXT m_debugMessenger;
};
