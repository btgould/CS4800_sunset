#include <vector>
#include <vulkan/vulkan.h>

class VulkanInstance {
  public:
	VulkanInstance();
	~VulkanInstance();
	void cleanup(); // TODO: This structure sucks

  private:
	void init();
	void createInstance();
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

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

	static bool checkValidationLayerSupport();
	static std::vector<const char*> getRequiredExtensions();

  private:
	static const std::vector<const char*> requiredValidationLayers;
#ifdef SUNSET_DEBUG
	static const bool enableValidationLayers = true;
#else
	static const bool enableValidationLayers = false;
#endif
};
