#include "texture.hpp"

#include "stb_image.h"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan.h>

Texture::Texture(std::string path, VulkanDevice& device) : m_device(device) {
	// load image from file
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels =
		stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	// save image to GPU memory
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                    stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

	// Free CPU memory
	stbi_image_free(pixels);

	// Create image object from buffer (optimized for shading)
	device.createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
	                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	// Copy buffer data into image object
	device.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
	                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	device.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth),
	                         static_cast<uint32_t>(texHeight));

	// Prepare image for shader access
	device.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
	                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Free staging buffer
	vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);
}

Texture::~Texture() {
	vkDestroyImage(m_device.getLogicalDevice(), textureImage, nullptr);
	vkFreeMemory(m_device.getLogicalDevice(), textureImageMemory, nullptr);
}
