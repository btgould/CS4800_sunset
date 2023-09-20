#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include <optional>

/* List of indices of queue families supporting a particular queue type, for some physical device.
 * All members are std::optional values. If they have a value, then it will be the index of a queue
 * family supporting that type of queue. Otherwise, no queue family for this devices supports that
 * type of queue.
 */
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	/* Checks if every queue type is supported by some queue family. */
	bool isComplete() { return graphicsFamily.has_value(); }
};

class VulkanInstance {
  public:
	VulkanInstance();
	~VulkanInstance();
	void cleanup(); // TODO: This structure sucks

  private:
	void init();
	void createInstance();
	static bool checkValidationLayerSupport();
	static std::vector<const char*> getRequiredExtensions();
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static VKAPI_ATTR VkBool32 VKAPI_CALL
	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	              VkDebugUtilsMessageTypeFlagsEXT messageType,
	              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	                                          VkDebugUtilsMessengerEXT debugMessenger,
	                                          const VkAllocationCallbacks* pAllocator);
	void pickPhysicalDevice();
	void createLogicalDevice();

  private:
	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;

	VkQueue m_graphicsQueue; // implicitly destroyed with logicalDevice

	VkDebugUtilsMessengerEXT m_debugMessenger;
	static const std::vector<const char*> requiredValidationLayers;
#ifdef SUNSET_DEBUG
	static const bool enableValidationLayers = true;
#else
	static const bool enableValidationLayers = false;
#endif
};
