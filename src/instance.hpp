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
	void setupDebugMessenger();
	void pickPhysicalDevice();
	void createLogicalDevice();

  private:
	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;

	VkQueue m_graphicsQueue; // implicitly destroyed with logicalDevice

	VkDebugUtilsMessengerEXT m_debugMessenger;
};
