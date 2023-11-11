#include "texture_lib.hpp"
#include "renderer/texture.hpp"
#include "util/memory.hpp"

TextureLibrary* TextureLibrary::s_instance;

TextureLibrary::TextureLibrary() {}

TextureLibrary* TextureLibrary::get() {
	if (s_instance == NULL) {
		s_instance = new TextureLibrary();
	}

	return s_instance;
}

Ref<Texture> TextureLibrary::getTexture(VulkanDevice& device, const std::string& filepath) {
	if (m_texMap.find(filepath) != m_texMap.end()) {
		return m_texMap[filepath];
	} else {
		Ref<Texture> tex = CreateRef<Texture>(filepath, device);
		m_texMap[filepath] = tex;
		return tex;
	}
}

void TextureLibrary::cleanup() {
	m_texMap.clear();
}
