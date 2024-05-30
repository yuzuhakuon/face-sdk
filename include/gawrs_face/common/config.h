#pragma once
#include <cstdint>

namespace gawrs_face
{
constexpr int kAlignedFaceSize = 112;
constexpr int kFeatureSize = 512;
constexpr int kPackedFeatureSize = kFeatureSize + 8;
constexpr int kFeatureByteSize = kFeatureSize * sizeof(float);
constexpr int kPackedFeatureByteSize = kPackedFeatureSize * sizeof(float);

constexpr uint8_t kFeatureVersionMajor = 0;
constexpr uint8_t kFeatureVersionMinor = 0;
constexpr uint8_t kFeatureVersionPatch = 1;
} // namespace gawrs_face