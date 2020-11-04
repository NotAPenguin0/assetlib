#include <assetlib/texture.hpp>
#include <json.hpp>
#include <lz4.h>

#include <cassert>

namespace assetlib {

//	Current texture parser version 0.0.1 has the following required fields:
//	format: a string containing the texture format. Has to be RGBA8
//	color_space: a string containing the color space. Has to be either SRGB or RGB.
//	extents: an object with 2 required fields
//		x: the width of the texture
//		y: the height of the texture
//	byte_size: the size in bytes of texture after decompression

static TextureFormat parse_texture_format(std::string const& fmt_string) {
	if (fmt_string == "RGBA8") { return TextureFormat::RGBA8; }
	return TextureFormat::Unknown;
}

static std::string format_to_string(TextureFormat format) {
	switch (format) {
	case TextureFormat::RGBA8:
		return "RGBA8";
	default:
		return "Unknown";
	}
}

static ColorSpace parse_color_space(std::string const& space) {
	if (space == "SRGB") { return ColorSpace::SRGB; }
	if (space == "RGB") { return ColorSpace::RGB; }
	return ColorSpace::Unknown;
}

static std::string colorspace_to_string(ColorSpace space) {
	switch (space) {
	case ColorSpace::SRGB:
		return "SRGB";
	case ColorSpace::RGB:
		return "RGB";
	default:
		return "Unknown";
	}
}

TextureInfo read_texture_info(AssetFile const& file) {
	// Verify version. TODO: proper error handling everywhere
	assert(file.version == itex_version && "file version mismatches parser version");

	TextureInfo info;
	json::JSON json = json::JSON::Load(file.metadata_json);

	std::string fmt_string = json["format"].ToString();
	info.format = parse_texture_format(fmt_string);
	json::JSON& extents = json["extents"];
	info.extents[0] = extents["x"].ToInt();
	info.extents[1] = extents["y"].ToInt();

	info.byte_size = json["byte_size"].ToInt();

	return info;
}

void unpack_texture(TextureInfo const& info, AssetFile const& file, void* dst) {
	// Decompress data directly into destination buffer
	int ret = LZ4_decompress_safe(file.binary_blob.data(), reinterpret_cast<char*>(dst), 
		file.binary_blob.size(), info.byte_size);
	assert(ret >= 0 && "decompression failed");
}

AssetFile pack_texture(TextureInfo const& info, void* pixel_data) {
	AssetFile file;

	json::JSON json;
	json["format"] = format_to_string(info.format);
	json["color_space"] = colorspace_to_string(info.color_space);
	json["extents"]["x"] = info.extents[0];
	json["extents"]["y"] = info.extents[1];
	json["byte_size"] = info.byte_size;

	// File header
	file.type[0] = 'I';
	file.type[1] = 'T';
	file.type[2] = 'E';
	file.type[3] = 'X';
	file.version = itex_version;
	file.metadata_json = json.dump(0, "");

	// No compression
	file.binary_blob.resize(info.byte_size);
	const int compress_staging_size = LZ4_compressBound(info.byte_size);
	file.binary_blob.resize(compress_staging_size);
	const int compressed_size = LZ4_compress_default(reinterpret_cast<const char*>(pixel_data), reinterpret_cast<char*>(file.binary_blob.data()), 
		info.byte_size, compress_staging_size);
	file.binary_blob.resize(compressed_size);


	return file;
}

}