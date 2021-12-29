#include <assetlib/environment.hpp>

#include <json.hpp>
#include <lz4.h>

#include <cassert>

namespace assetlib {

EnvironmentInfo read_environment_info(AssetFile const& file) {
    assert(file.type[0] == 'I' && file.type[1] == 'E' && file.type[2] == 'N' && file.type[3] == 'V' && file.version == ienv_version && "Type/version mismatch");

    EnvironmentInfo info;
    json::JSON json = json::JSON::Load(file.metadata_json);
    info.compression = parse_compression_mode(json["compression_mode"].ToString());
    info.hdr_extents[0] = json["hdr_extents"]["x"].ToInt();
    info.hdr_extents[1] = json["hdr_extents"]["y"].ToInt();
    info.hdr_bytes = json["hdr_bytes"].ToInt();
    info.irradiance_size = json["irradiance_size"].ToInt();
    info.irradiance_bytes = json["irradiance_bytes"].ToInt();
    info.irradiance_offset = json["irradiance_offset"].ToInt();
    info.specular_size = json["specular_size"].ToInt();
    info.specular_bytes = json["specular_bytes"].ToInt();
    info.specular_offset = json["specular_offset"].ToInt();
    return info;
}

void unpack_environment(EnvironmentInfo const& info, AssetFile const& file, void* dst_hdr, void* dst_irradiance, void* dst_specular) {
    char const* start = file.binary_blob.data();
    if (info.compression == CompressionMode::LZ4) {
        LZ4_decompress_safe(start, reinterpret_cast<char*>(dst_hdr), info.irradiance_offset, info.hdr_bytes);
        LZ4_decompress_safe(start + info.irradiance_offset, reinterpret_cast<char*>(dst_irradiance), info.specular_offset - info.irradiance_offset, info.irradiance_bytes);
        LZ4_decompress_safe(start + info.specular_offset, reinterpret_cast<char*>(dst_specular), file.binary_blob.size() - info.specular_offset, info.specular_bytes);
    } else { // No compression
        std::memcpy(dst_hdr, start, info.hdr_bytes);
        std::memcpy(dst_irradiance, start + info.irradiance_offset, info.irradiance_bytes);
        std::memcpy(dst_specular, start + info.specular_offset, info.specular_bytes);
    }
}

AssetFile pack_environment(EnvironmentInfo const& info, void* hdr, void* irradiance, void* specular) {
    AssetFile file{};
    file.version = ienv_version;
    file.type[0] = 'I'; file.type[1] = 'E'; file.type[2] = 'N'; file.type[3] = 'V';

    json::JSON json{};
    json["compression_mode"] = compression_to_string(CompressionMode::LZ4);
    json["hdr_extents"]["x"] = info.hdr_extents[0]; json["hdr_extents"]["y"] = info.hdr_extents[1];
    json["hdr_bytes"] = info.hdr_bytes;
    json["irradiance_size"] = info.irradiance_size;
    json["irradiance_bytes"] = info.irradiance_bytes;
    json["irradiance_offset"] = info.hdr_bytes;
    json["specular_size"] = info.specular_size;
    json["specular_bytes"] = info.specular_bytes;
    json["specular_offset"] = info.irradiance_offset + info.irradiance_bytes;

    // Compress the data pointers into the final binary blob
    int staging_size = LZ4_compressBound(info.hdr_bytes);
    file.binary_blob.resize(staging_size);
    int compressed_size = LZ4_compress_default(reinterpret_cast<const char*>(hdr), file.binary_blob.data(), info.hdr_bytes, staging_size);
    int offset = compressed_size;
    file.binary_blob.resize(compressed_size);
    // Compress again, this time at offset compressed_size in the file binary blob
    staging_size = LZ4_compressBound(info.irradiance_bytes);
    file.binary_blob.resize(offset + staging_size);
    int compressed_irr_size = LZ4_compress_default(reinterpret_cast<const char*>(irradiance), file.binary_blob.data() + offset, info.irradiance_bytes, staging_size);
    file.binary_blob.resize(compressed_irr_size + offset);
    offset += compressed_irr_size;
    // And again for specular, but now at offset compressed_irr_size + compressed_size
    staging_size = LZ4_compressBound(info.specular_bytes);
    file.binary_blob.resize(offset + staging_size);
    int compressed_specular_size = LZ4_compress_default(reinterpret_cast<const char*>(specular), file.binary_blob.data() + offset, info.specular_bytes, staging_size);
    file.binary_blob.resize(compressed_specular_size + offset);

    file.metadata_json = json.dump(0, "");
    return file;
}

}