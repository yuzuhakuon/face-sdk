#pragma once

namespace gawrs_face
{
constexpr int kAlignedFaceSize = 112;
constexpr int kFeatureSize = 512;
constexpr int kPackedFeatureSize = kFeatureSize + 8;
constexpr int kFeatureByteSize = kFeatureSize * sizeof(float);
constexpr int kPackedFeatureByteSize = kPackedFeatureSize * sizeof(float);
} // namespace gawrs_face