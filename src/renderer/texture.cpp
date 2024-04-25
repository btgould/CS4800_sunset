#include "texture.hpp"

#include "stb_image.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

Texture::Texture(Ref<VulkanDevice> device, const glm::uvec2& size, VkFormat imageFormat,
                 TextureAccessBitFlag accessType, bool depth)
	: m_device(device), m_size(size), m_numChannels(4) {
	VkDeviceSize imageSize = m_size.x * m_size.y * 4;
	VkImageUsageFlags usageFlag = 0;
	if ((accessType | TextureAccessBitFlag::READ_BIT) != 0)
		usageFlag |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if ((accessType | TextureAccessBitFlag::READ_BIT) != 0)
		usageFlag |= VK_IMAGE_USAGE_SAMPLED_BIT;
	VkImageAspectFlags type = depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (depth)
		usageFlag = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	// Create image object from buffer (optimized for shading)
	m_device->createImage(m_size.x, m_size.y, imageFormat, VK_IMAGE_TILING_OPTIMAL, usageFlag,
	                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
	m_imageView = m_device->createImageView(m_image, imageFormat, type);
}

Texture::Texture(std::string path, Ref<VulkanDevice> device) : m_device(device) {
	createTextureImage(path);
	m_imageView =
		m_device->createImageView(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

Texture::~Texture() {
	vkDestroyImageView(m_device->getLogicalDevice(), m_imageView, nullptr);
	vkDestroyImage(m_device->getLogicalDevice(), m_image, nullptr);
	vkFreeMemory(m_device->getLogicalDevice(), m_imageMemory, nullptr);
}

void Texture::createTextureImage(std::string path) {
	stbi_uc* pixels;
	// load image from file
	pixels = stbi_load(path.c_str(), (int*) &m_size.x, (int*) &m_size.y, (int*) &m_numChannels,
	                   STBI_rgb_alpha);
	VkDeviceSize imageSize = m_size.x * m_size.y * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	// save image to GPU memory
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_device->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                       stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(m_device->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device->getLogicalDevice(), stagingBufferMemory);

	// Free CPU memory
	stbi_image_free(pixels);

	// Create image object from buffer (optimized for shading)
	m_device->createImage(m_size.x, m_size.y, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
	                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
	// Copy buffer data into image object
	transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
	                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(m_size.x),
	                  static_cast<uint32_t>(m_size.y));
	// NOTE: this interface could be cleaned up

	// Prepare image for shader access
	transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Free staging buffer
	vkDestroyBuffer(m_device->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_device->getLogicalDevice(), stagingBufferMemory, nullptr);
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                    VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex =
		VK_QUEUE_FAMILY_IGNORED; // not transferring queue ownership, just layout
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
	    newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		// transfer does not need to wait on anything
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
	           newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		// fragment shader needs to wait on transfer finishing
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
	                     &barrier);

	m_device->endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	VkBufferImageCopy region {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
	                       &region);

	m_device->endSingleTimeCommands(commandBuffer);
}
