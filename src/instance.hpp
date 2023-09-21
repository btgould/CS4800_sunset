#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include <optional>

#include "window.hpp"

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

class VulkanInstance {
  public:
	VulkanInstance(GLFWWindow& window);
	~VulkanInstance();

  private: // core interface
	void init();
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();

  private: // validation / debug
	static const std::vector<const char*> requiredValidationLayers;

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

  private: // device selection
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

  private:
	GLFWWindow m_window;

	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;

	VkSurfaceKHR m_surface;

	VkQueue m_graphicsQueue; // implicitly destroyed with logicalDevice
	VkQueue m_presentQueue;

	VkDebugUtilsMessengerEXT m_debugMessenger;
};
