#include <assetlib/asset_file.hpp>

#include <plib/stream.hpp>

namespace assetlib {

std::string compression_to_string(CompressionMode compression) {
	switch (compression) {
	case CompressionMode::None:
		return "None";
	case CompressionMode::LZ4:
		return "LZ4";
	}
}

CompressionMode parse_compression_mode(std::string_view compression) {
	if (compression == "LZ4") { return CompressionMode::LZ4; }
	return CompressionMode::None;
}

bool save_binary_file(plib::binary_output_stream& out, AssetFile const& file) {
	// Write metadata
	out.write(file.type, sizeof(file.type));
	uint32_t version = file.version;
	out.write(&version, 1);

	uint32_t json_length = file.metadata_json.size();
	out.write(&json_length, 1);

	uint32_t binary_length = file.binary_blob.size();
	out.write(&binary_length, 1);

	// Write json
	out.write(file.metadata_json.data(), json_length);
	// Write binary blob
	out.write(file.binary_blob.data(), binary_length);

	return true;
}

bool load_binary_file(plib::binary_input_stream& in, AssetFile& file) {
	in.read(file.type, sizeof(file.type));
	in.read(&file.version, 1);
	uint32_t json_length, binary_length;
	in.read(&json_length, 1);
	in.read(&binary_length, 1);

	file.metadata_json.resize(json_length);
	file.binary_blob.resize(binary_length);

	in.read(file.metadata_json.data(), json_length);
	in.read(file.binary_blob.data(), binary_length);
	return true;
}

}