#pragma once

enum RuntimeModel
{
    GF_NONE = 0,
    GF_FACE_DETECTION = 0b00000001,
    GF_FACE_RECOGNITION = 0b00000010,
};

enum RotationModel
{
    GF_ROTATE_0 = 0,
    GF_ROTATE_90,
    GF_ROTATE_180,
    GF_ROTATE_270,
    GF_HorizontalFlip,
    GF_VerticalFlip,
    GF_RotationFlip,
    GF_FlipRotation,
};

enum ImageFormat
{
    // The format is unknown. It is not valid for an ImageFrame to be
    // initialized with this value.
    GF_UNKNOWN = 0,

    // BGR888, interleaved: one byte for B, then one byte for G, then one
    // byte for R for each pixel.
    GF_BGR24 = 1,

    // RGB888, interleaved: one byte for R, then one byte for G, then one
    // byte for B for each pixel.
    GF_RGB24 = 2,

    // Grayscale, one byte per pixel.
    GF_GRAY = 3,

    // sBGRA, interleaved: one byte for B, one byte for G, one byte for R,
    // one byte for alpha or unused. This is the N32 format for Skia.
    GF_BGRA = 4,

    // sRGBA, interleaved: one byte for R, one byte for G, one byte for B,
    // one byte for alpha or unused.
    GF_RGBA = 5,
};