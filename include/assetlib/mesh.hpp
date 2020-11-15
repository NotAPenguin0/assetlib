#pragma once

#include <assetlib/asset_file.hpp>

namespace assetlib {

enum class VertexFormat {
	// Position (3) - Normal (3) - Tangent (3) - UV (2), all 32-bit float
	PNTV32,
	Unknown
};

struct PNTV32Vertex {
	float position[3]{};
	float normal[3]{};
	float tangent[3]{};
	float uv[2]{};
};

struct MeshInfo {
	VertexFormat format = VertexFormat::PNTV32;
	CompressionMode compression = CompressionMode::LZ4;
	uint32_t vertex_count = 0;
	uint32_t index_count = 0;
	uint8_t index_bits = 32;
	// This does not need to be set when packing a mesh
	uint32_t index_binary_offset = 0;
};

MeshInfo read_mesh_info(AssetFile const& file);

// Unpacks raw mesh data into destination buffers
void unpack_mesh(MeshInfo const& info, AssetFile const& file, void* dst_vertices, void* dst_indices);

// Packs raw mesh data into a binary asset file ready to save to disk
AssetFile pack_mesh(MeshInfo const& info, void* vertices, void* indices);

}