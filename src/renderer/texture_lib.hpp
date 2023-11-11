#pragma once

#include <map>
#include <string>
#include <unordered_map>

#include "bootstrap/device.hpp"
#include "renderer/texture.hpp"
#include "util/memory.hpp"

class TextureLibrary {
  private:
	static TextureLibrary* s_instance;

	TextureLibrary();
	~TextureLibrary() = default;

	TextureLibrary(const TextureLibrary&) = delete;
	TextureLibrary& operator=(const TextureLibrary&) = delete;

  public:
	static TextureLibrary* get();
	Ref<Texture> getTexture(VulkanDevice& device, const std::string& filepath);
	void cleanup();

  private:
	std::map<std::string, Ref<Texture>> m_texMap;
};
