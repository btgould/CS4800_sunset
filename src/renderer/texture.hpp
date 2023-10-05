#pragma once

#include "bootstrap/device.hpp"
#include <string>

class Texture {
  public:
	Texture(std::string path, VulkanDevice& device);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

  private:
	VulkanDevice& m_device;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
};
