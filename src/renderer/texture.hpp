#pragma once

#include "bootstrap/device.hpp"

#include <string>

class Texture {
  public:
	Texture(std::string path, VulkanDevice& device);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	inline VkImageView getImageView() const { return m_textureImageView; }
	inline uint32_t getID() const { return m_id; }

  private: // core interface
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
	VulkanDevice& m_device;

	VkImage m_textureImage;
	VkImageView m_textureImageView;
	VkDeviceMemory m_textureImageMemory;

	uint32_t m_id;

	static uint32_t count;
};
