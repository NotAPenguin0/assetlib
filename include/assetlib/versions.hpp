#pragma once 

#include <cstdint>

namespace assetlib {

// Versioning will be as follows:
// A version can be seen as major.minor.patch
// The lower 8 bits describe the patch version
// The next 8 bits are the minor version
// The next 8 bits are the major version
// The highest 8 bits are reserved for future use.

inline constexpr uint32_t pack_version(uint8_t major, uint8_t minor, uint8_t patch) {
	uint32_t result = patch;
	result += (minor << 8);
	result += (major << 16);
	return result;
}

inline constexpr uint8_t major_version(uint32_t version) {
	return version >> 16;
}

inline constexpr uint8_t minor_version(uint32_t version) {
	return version >> 8;
}

inline constexpr uint8_t patch_version(uint32_t version) {
	return version & 0xFF;
}

constexpr uint32_t itex_version = pack_version(1, 0, 0);
constexpr uint32_t mesh_version = pack_version(0, 0, 1);

}