#pragma once
#include <cstdint>

namespace gawrs_face
{
enum class ImageFormat : uint32_t
{
    // The format is unknown. It is not valid for an ImageFrame to be
    // initialized with this value.
    UNKNOWN = 0,

    // BGR888, interleaved: one byte for B, then one byte for G, then one
    // byte for R for each pixel.
    BGR24 = 1,

    // RGB888, interleaved: one byte for R, then one byte for G, then one
    // byte for B for each pixel.
    RGB24 = 2,

    // Grayscale, one byte per pixel.
    GRAY = 3,

    // sBGRA, interleaved: one byte for B, one byte for G, one byte for R,
    // one byte for alpha or unused. This is the N32 format for Skia.
    BGBA = 4,

    // sRGBA, interleaved: one byte for R, one byte for G, one byte for B,
    // one byte for alpha or unused.
    RGBA = 5,

    // NV12 is a biplanar format with a full sized Y plane followed by
    // a single chroma plane with weaved U and V values
    NV12 = 6,

    // NV21 is a biplanar format with a full sized Y plane followed by
    // a single chroma plane with weaved V and U values
    NV21 = 7,
};
}