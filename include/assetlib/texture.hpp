#pragma once

#include <assetlib/asset_file.hpp>
#include <cstddef>

namespace assetlib {

enum class TextureFormat {
	Unknown = 0,
	RGBA8
};

enum class ColorSpace {
	Unknown = 0,
	SRGB,
	RGB
};

struct TextureInfo {
	uint64_t byte_size = 0;
	TextureFormat format = TextureFormat::Unknown;
	ColorSpace color_space = ColorSpace::Unknown;
	uint32_t extents[3]{ 0, 0, 0 };
};

// Read texture metadata from binary file
TextureInfo read_texture_info(AssetFile const& file);

// Unpacks raw pixel data into destination buffer
void unpack_texture(TextureInfo const& info, AssetFile const& file, std::byte* dst);

// Packs raw pixel data into a binary asset file ready to save to disk
AssetFile pack_texture(TextureInfo const& info, std::byte* pixel_data);

}