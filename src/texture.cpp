#include <assetlib/texture.hpp>
#include <json.hpp>
#include <lz4.h>

#include <cassert>

namespace assetlib {

//	Current texture parser version 1.0.1 has the following required fields:
//	format: a string containing the texture format. Has to be RGBA8
//	extents: an object with 2 required fields
//		x: the width of the texture
//		y: the height of the texture
//	byte_size: the size in bytes of texture after decompression
//	compression_mode: String with the compression mode used. "None" for no compression, "LZ4" for LZ4 compression.
//	mip_levels: amount of mip levels stored in the file.
// Following fields are optional
// color_space: a string containing the color space. Has to be either sRGB or RGB. (default value is RGB)

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
	if (space == "sRGB") { return ColorSpace::sRGB; }
	if (space == "RGB") { return ColorSpace::RGB; }
	return ColorSpace::Unknown;
}

static std::string colorspace_to_string(ColorSpace space) {
	switch (space) {
	case ColorSpace::sRGB:
		return "sRGB";
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

	info.format = parse_texture_format(json["format"].ToString());
	info.compression = parse_compression_mode(json["compression_mode"].ToString());
	json::JSON& extents = json["extents"];
	info.extents[0] = extents["x"].ToInt();
	info.extents[1] = extents["y"].ToInt();
	info.byte_size = json["byte_size"].ToInt();
	info.mip_levels = json["mip_levels"].ToInt();

    if (json.hasKey("color_space")) info.colorspace = parse_color_space(json["colorspace"].ToString());
    else info.colorspace = ColorSpace::RGB;

	return info;
}

void unpack_texture(TextureInfo const& info, AssetFile const& file, void* dst) {
	if (info.compression == CompressionMode::LZ4) {
		// Decompress data directly into destination buffer
		LZ4_decompress_safe(file.binary_blob.data(), reinterpret_cast<char*>(dst),
			file.binary_blob.size(), info.byte_size);
	}
	else if (info.compression == CompressionMode::None) {
		memcpy(dst, file.binary_blob.data(), file.binary_blob.size());
	}
}

AssetFile pack_texture(TextureInfo const& info, void* pixel_data) {
	AssetFile file;

	json::JSON json;
	json["format"] = format_to_string(info.format);
	json["extents"]["x"] = info.extents[0];
	json["extents"]["y"] = info.extents[1];
	json["byte_size"] = info.byte_size;
	json["mip_levels"] = info.mip_levels;
    json["color_space"] = info.colorspace;

	// File header
	file.type[0] = 'I';
	file.type[1] = 'T';
	file.type[2] = 'E';
	file.type[3] = 'X';
	file.version = itex_version;

	CompressionMode compression = info.compression;
	if (compression == CompressionMode::LZ4) {

		const int compress_staging_size = LZ4_compressBound(info.byte_size);
		file.binary_blob.resize(compress_staging_size);
		const int compressed_size = LZ4_compress_default(reinterpret_cast<const char*>(pixel_data), file.binary_blob.data(),
			info.byte_size, compress_staging_size);
		file.binary_blob.resize(compressed_size);

		const float compression_ratio = (float)compressed_size / (float)info.byte_size;
		// Compression ratio of > 80% is not worth it
		if (compression_ratio > 0.8) {
			compression = CompressionMode::None;
		}
	}
	// No else, because the compression mode can change because of the previous if
	if (compression == CompressionMode::None) {
		// Use a raw pixel dump instead (memcpy)
		file.binary_blob.resize(info.byte_size);
		memcpy(file.binary_blob.data(), pixel_data, info.byte_size);
	}

	json["compression_mode"] = compression_to_string(compression);

	file.metadata_json = json.dump(0, "");

	return file;
}

}