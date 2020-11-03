#include <assetlib/asset_file.hpp>

#include <fstream>

namespace assetlib {

bool save_binary_file(std::string_view path, AssetFile const& file) {
	std::ofstream out(path.data(), std::ios::out | std::ios::binary);
	if (!out.good()) return false;

	// Write metadata
	out.write(file.type, sizeof(file.type));
	uint32_t version = file.version;
	out.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));

	uint32_t json_length = file.metadata_json.size();
	out.write(reinterpret_cast<const char*>(&json_length), sizeof(uint32_t));

	uint32_t binary_length = file.binary_blob.size();
	out.write(reinterpret_cast<const char*>(&binary_length), sizeof(uint32_t));

	// Write json
	out.write(file.metadata_json.data(), json_length);
	// Write binary blob
	out.write(file.binary_blob.data(), binary_length);

	return true;
}

bool load_binary_file(std::string_view path, AssetFile& file) {
	std::ifstream in(path.data(), std::ios::in, std::ios::binary);
	if (!in.good()) return false;

	in.read(file.type, sizeof(file.type));
	in.read(reinterpret_cast<char*>(&file.version), sizeof(uint32_t));
	uint32_t json_length, binary_length;
	in.read(reinterpret_cast<char*>(&json_length), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&binary_length), sizeof(uint32_t));

	file.metadata_json.resize(json_length);
	file.binary_blob.resize(binary_length);

	in.read(file.metadata_json.data(), json_length);
	in.read(file.binary_blob.data(), binary_length);
	return true;
}

}