#include "instance.hpp"

#include <cstring>
#include <GLFW/glfw3.h>
#include <type_traits>
#include <vector>

#include "util/log.hpp"

const std::vector<const char*> VulkanInstance::requiredValidationLayers = {
	"VK_LAYER_KHRONOS_validation"};

VulkanInstance::VulkanInstance() {
	init();
}

VulkanInstance::~VulkanInstance() {}

void VulkanInstance::init() {
	createInstance();
	setupDebugMessenger();
	pickPhysicalDevice();
}

void VulkanInstance::cleanup() {
	vkDestroyDevice(m_logicalDevice, nullptr);
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroyInstance(m_instance, nullptr);
}

void VulkanInstance::createInstance() {
	// Ensure required validation layers are available
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Create application and vulkan instance
	VkApplicationInfo appInfo {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
		createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	// get list of graphics extensions required to use glfw
	auto extensions = getRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	for (const auto& extension : extensions) {
		LOG_TRACE("Required extension: {0}", extension);
	}

	// get list of extensions supported by vulkan instillation
	uint32_t supportedExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount,
	                                       supportedExtensions.data());

	LOG_TRACE("Available Extensions: ");
	for (const auto& extension : supportedExtensions) {
		LOG_TRACE("\t{0}", extension.extensionName);
	}

	// check that required extensions are all supported
	for (const auto& extension : extensions) {
		bool extensionSupported = false;

		for (const auto& supportedExtension : supportedExtensions) {
			if (strcmp(extension, supportedExtension.extensionName) != 0) {
				extensionSupported = true;
			}
		}

		if (!extensionSupported) {
			throw std::runtime_error("Required extension not supported!");
		}
	}

	// set up validation for instance creation
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
		createInfo.ppEnabledLayerNames = requiredValidationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	// create vulkan instance
	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Vulkan instance!");
	}
}

bool VulkanInstance::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : requiredValidationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> VulkanInstance::getRequiredExtensions() {
	// TODO: what are graphics extensions here? do they belong to GPU?
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanInstance::setupDebugMessenger() {
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo {};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(
	VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanInstance::populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkBool32 VulkanInstance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                       void* pUserData) {
	const char* descString;
	switch (messageType) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		descString = "Validation info: ";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		descString = "Validation issue: ";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		descString = "Performance issue: ";
		break;
	}

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		LOG_TRACE("{0} {1}", descString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		LOG_INFO("{0} {1}", descString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOG_WARN("{0} {1}", descString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		LOG_ERROR("{0} {1}", descString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
		break;
	}

	return VK_FALSE;
}

void VulkanInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                   VkDebugUtilsMessengerEXT debugMessenger,
                                                   const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
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

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
	// Query device features / properties
	// VkPhysicalDeviceProperties deviceProperties;
	// VkPhysicalDeviceFeatures deviceFeatures;
	// vkGetPhysicalDeviceProperties(device, &deviceProperties);
	// vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
	// deviceFeatures.geometryShader;
	QueueFamilyIndices indices = findQueueFamilies(device);

	return indices.isComplete();
}

void VulkanInstance::pickPhysicalDevice() {
	m_physicalDevice = VK_NULL_HANDLE;

	// find all available physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	// select first suitable physical device
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void VulkanInstance::createLogicalDevice() {
	float queuePriority = 1.0f;
	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

	// Create graphics queue
	VkDeviceQueueCreateInfo queueCreateInfo {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// Create logical device
	VkPhysicalDeviceFeatures deviceFeatures {};

	VkDeviceCreateInfo deviceCreateInfo {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = 0;

	if (enableValidationLayers) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = requiredValidationLayers.data();
	} else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// Save handle to graphics queue
	// We can use 0 here currently because we only need a graphics queue
	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
}
