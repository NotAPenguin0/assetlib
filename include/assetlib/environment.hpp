#pragma once

#include <assetlib/asset_file.hpp>

namespace assetlib {

struct EnvironmentInfo {
    // ignored by pack_environment(). LZ4 will always be used
    CompressionMode compression = CompressionMode::None;
    uint32_t hdr_extents[2] { 0, 0 };
    uint32_t hdr_bytes = 0;
    uint32_t irradiance_size = 0;
    uint32_t irradiance_bytes = 0;
    // no need to set this manually before calling pack_environment()
    uint32_t irradiance_offset = 0;
    uint32_t specular_size = 0;
    uint32_t specular_bytes = 0;
    // no need to set this manually before calling pack_environment()
    uint32_t specular_offset = 0;
};

// Read environment info from an asset file
EnvironmentInfo read_environment_info(AssetFile const& file);

// Unpack environment into destination buffers
void unpack_environment(EnvironmentInfo const& info, AssetFile const& file, void* dst_hdr, void* dst_irradiance, void* dst_specular);

// Packs raw pixel data into a binary asset file ready to save to disk
AssetFile pack_environment(EnvironmentInfo const& info, void* hdr, void* irradiance, void* specular);

}