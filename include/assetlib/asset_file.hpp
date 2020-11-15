#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <assetlib/versions.hpp>

namespace assetlib {

enum class CompressionMode {
	None,
	LZ4
};

std::string compression_to_string(CompressionMode compression);
CompressionMode parse_compression_mode(std::string_view compression);

struct AssetFile {
	// 4-byte header indicating the asset type.
	// Possible values are:
	// ITEX for textures
	// MESH for meshes
	char type[4];
	// Version of the binary file format.
	uint32_t version;
	// json description of the asset
	std::string metadata_json;
	// Binary buffer with all the raw data
	std::vector<char> binary_blob;
};

bool save_binary_file(std::string_view path, AssetFile const& file);
bool load_binary_file(std::string_view path, AssetFile& file);

}