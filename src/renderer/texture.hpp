#pragma once

#include "bootstrap/device.hpp"

#include <glm/glm.hpp>
#include <string>

class Texture {
  public:
	Texture(const glm::uvec2& size, Ref<VulkanDevice> device);
	Texture(std::string path, Ref<VulkanDevice> device);
	~Texture();

	Texture(const Texture&) = delete;

	inline VkImageView getImageView() const { return m_imageView; }

  private: // core interface
	/**
	 * @brief Creates a read-only texture from an image on disk
	 *
	 * @param path Relative path to an image file to load
	 */
	void createTextureImage(std::string path);

  private: // helper functions
	/**
	 * @brief Changes the layout of the given image from oldLayout to newLayout
	 *
	 * @param image The image to transition
	 * @param format The data types used to represent each texel of the image
	 * @param oldLayout The previous ordering of image texels in memory
	 * @param newLayout The desired ordering of image texels in memory
	 */
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
	                           VkImageLayout newLayout);

	/**
	 * @brief Copies a buffer object into an image object
	 *
	 * @param buffer The buffer to copy data from
	 * @param image The image to copy data into
	 * @param width The width of the image to copy
	 * @param height The height of the image to copy
	 */
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

  private:
	glm::uvec2 m_size;
	uint32_t m_numChannels;

  private:
	Ref<VulkanDevice> m_device;

	VkImage m_image;
	VkImageView m_imageView;
	VkDeviceMemory m_imageMemory;
};
