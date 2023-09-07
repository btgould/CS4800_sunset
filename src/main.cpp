#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <string.h>
#include <stdexcept>
#include <cstdlib>

#include "util/log.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
  public:
	void run() {
		Log::Init(spdlog::level::trace);

		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

  private:
	GLFWwindow* m_window;
	VkInstance m_instance;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() { createInstance(); }

	void createInstance() {
		VkApplicationInfo appInfo {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// get list of graphics extensions required to use glfw
		// TODO: what are graphics extensions here? do they belong to GPU?
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		for (uint32_t i = 0; i < glfwExtensionCount; i++) {
			LOG_TRACE("Required extension: {0}", glfwExtensions[i]);
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
		for (uint32_t i = 0; i < glfwExtensionCount; i++) {
			bool extensionSupported = false;

			for (const auto& supportedExtension : supportedExtensions) {
				if (strcmp(glfwExtensions[i], supportedExtension.extensionName) != 0) {
					extensionSupported = true;
				}
			}

			if (!extensionSupported) {
				throw std::runtime_error("Required extension not supported!");
			}
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
