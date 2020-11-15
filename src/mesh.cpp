#include <assetlib/mesh.hpp>
#include <json.hpp>
#include <lz4.h>

#include <cassert>

namespace assetlib {

//	Current mesh parser version 0.0.1 has the following required fields:
//	vertex_count: unsigned integer holding the number of vertices
//	index_count: unsigned integer holding the number of indices
//	index_bits: either 16 or 32, indicating how large one index is. Indices are unsigned integers of this width
//	index_binary_offset: offset in the binary blob where indices start
//	vertex_format: a string describing the vertex format. Must be one of the following
//		- PNTV32: Position (3) - Normal (3) - Tangent (3) - UV (2), all 32-bit float
//	compression_mode: compression mode used when packing the asset file. Must be None or LZ4
//	TODO: Add field for mesh boundaries

static VertexFormat parse_vertex_format(std::string const& format) {
	if (format == "PNTV32") return VertexFormat::PNTV32;
	return VertexFormat::Unknown;
}

static std::string format_to_string(VertexFormat format) {
	switch (format) {
	case VertexFormat::PNTV32:
		return "PNTV32";
	default:
		return "Unknown";
	}
}

static uint32_t vertex_byte_size(VertexFormat format) {
	switch (format) {
	case VertexFormat::PNTV32:
		// Position + Normal + Tangent + UV
		return (3 + 3 + 3 + 2) * sizeof(float);
	default:
		return 0;
	}
}

MeshInfo read_mesh_info(AssetFile const& file) {
	assert(file.version == mesh_version && "file version mismatches parser version");

	MeshInfo info{};
	json::JSON json = json::JSON::Load(file.metadata_json);

	info.vertex_count = json["vertex_count"].ToInt();
	info.index_count = json["index_count"].ToInt();
	info.index_bits = json["index_bits"].ToInt();
	info.index_binary_offset = json["index_binary_offset"].ToInt();
	info.format = parse_vertex_format(json["vertex_format"].ToString());
	info.compression = parse_compression_mode(json["compression_mode"].ToString());

	return info;
}

// Unpacks raw mesh data into destination buffers
void unpack_mesh(MeshInfo const& info, AssetFile const& file, void* dst_vertices, void* dst_indices) {
	uint32_t bytes_per_index = info.index_bits / 8;
	const char* src_vertex_pointer = file.binary_blob.data();
	const char* src_index_pointer = file.binary_blob.data() + info.index_binary_offset;
	if (info.compression == CompressionMode::LZ4) {
		// info.index_binary_offset is the lenght of the vertex data in the compressed binary stream
		LZ4_decompress_safe(src_vertex_pointer, reinterpret_cast<char*>(dst_vertices), 
			info.index_binary_offset, info.vertex_count * vertex_byte_size(info.format));
		LZ4_decompress_safe(src_index_pointer, reinterpret_cast<char*>(dst_indices), 
			file.binary_blob.size() - info.index_binary_offset, info.index_count * bytes_per_index);
	}
	else if (info.compression == CompressionMode::None) {
		memcpy(dst_vertices, src_vertex_pointer, info.vertex_count * vertex_byte_size(info.format));
		memcpy(dst_indices, src_index_pointer, info.index_count * bytes_per_index);
	}
}

template<typename T, typename... Us>
bool any_of(T val, Us... others) {
	return ((val == others) || ...);
}

static bool validate_mesh_info(MeshInfo const& info) {
	if (!any_of(info.index_bits, 16, 32)) return false;
	if (!any_of(info.compression, assetlib::CompressionMode::None, assetlib::CompressionMode::LZ4)) return false;
	if (!any_of(info.format, assetlib::VertexFormat::PNTV32)) return false;
	if (info.index_count == 0) return false;
	if (info.vertex_count == 0) return false;
	return true;
}

// Packs raw mesh data into a binary asset file ready to save to disk
AssetFile pack_mesh(MeshInfo const& info, void* vertices, void* indices) {
	AssetFile file;
	
	assert(validate_mesh_info(info) && "Invalid mesh description");

	json::JSON json;
	json["vertex_count"] = info.vertex_count;
	json["index_count"] = info.index_count;
	json["index_bits"] = info.index_bits;
	json["vertex_format"] = format_to_string(info.format);
	json["compression_mode"] = compression_to_string(info.compression);

	uint32_t const vtx_byte_size = info.vertex_count * vertex_byte_size(info.format);
	uint32_t const idx_byte_size = info.index_count * (info.index_bits / 8);
	if (info.compression == CompressionMode::LZ4) {
		// Compress vertex buffer
		uint32_t compress_working_size = LZ4_compressBound(vtx_byte_size);
		file.binary_blob.resize(compress_working_size);
		uint32_t compressed_vtx_size = LZ4_compress_default(reinterpret_cast<const char*>(vertices), file.binary_blob.data(), vtx_byte_size, compress_working_size);
		// The index offset is now compressed_vtx_size bytes
		json["index_binary_offset"] = compressed_vtx_size;
		// Compress indices
		compress_working_size = LZ4_compressBound(idx_byte_size);
		file.binary_blob.resize(compressed_vtx_size + compress_working_size);
		uint32_t compressed_idx_size = LZ4_compress_default(reinterpret_cast<const char*>(indices), file.binary_blob.data() + compressed_vtx_size, idx_byte_size, compress_working_size);
		// Resize binary blob down to correct size
		file.binary_blob.resize(compressed_vtx_size + compressed_idx_size);
		file.binary_blob.shrink_to_fit();
	}
	else if (info.compression == CompressionMode::None) {
		// No compression, just two raw memcpy() calls
		file.binary_blob.resize(vtx_byte_size + idx_byte_size);
		memcpy(file.binary_blob.data(), reinterpret_cast<const char*>(vertices), vtx_byte_size);
		memcpy(file.binary_blob.data() + vtx_byte_size, reinterpret_cast<const char*>(indices), idx_byte_size);
	}

	file.metadata_json = json.dump(0, "");

	return file;
}


}