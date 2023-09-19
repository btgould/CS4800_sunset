#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>
#include <stdexcept>
#include <cstdlib>

#include "util/log.hpp"
#include "engine.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> requiredValidationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef SUNSET_DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

class HelloTriangleApplication {
  public:
	void run() {
		Log::Init(spdlog::level::info);

		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

  private:
	GLFWwindow* m_window;
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo {};
		populateDebugMessengerCreateInfo(createInfo);

		if (VulkanEngine::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr,
		                                               &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanEngine::debugCallback;
	}

	void createInstance() {
		// Ensure required validation layers are available
		if (enableValidationLayers && !VulkanEngine::checkValidationLayerSupport()) {
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
		auto extensions = VulkanEngine::getRequiredExtensions();

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
		//
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

	void mainLoop() {
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {

		if (enableValidationLayers) {
			VulkanEngine::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}

		vkDestroyInstance(m_instance, nullptr);

		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::exception& e) {
		LOG_ERROR("{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
