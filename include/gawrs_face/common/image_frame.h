#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

// reference: https://github.com/google/mediapipe/blob/master/mediapipe/framework/formats/image_frame.h

namespace gawrs_face
{
enum class ImageFormat : int
{
    // The format is unknown.  It is not valid for an ImageFrame to be
    // initialized with this value.
    UNKNOWN = 0,

    // sRGB, interleaved: one byte for R, then one byte for G, then one
    // byte for B for each pixel.
    SRGB = 1,

    // sRGBA, interleaved: one byte for R, one byte for G, one byte for B,
    // one byte for alpha or unused.
    SRGBA = 2,

    // Grayscale, one byte per pixel.
    GRAY8 = 3,

    // Grayscale, one uint16 per pixel.
    GRAY16 = 4,

    // YCbCr420P (1 bpp for Y, 0.25 bpp for U and V).
    // NOTE: NOT a valid ImageFrame format, but intended for
    // ScaleImageCalculatorOptions, VideoHeader, etc. to indicate that
    // YUVImage is used in place of ImageFrame.
    YCBCR420P = 5,

    // Similar to YCbCr420P, but the data is represented as the lower 10bits of
    // a uint16. Like YCbCr420P, this is NOT a valid ImageFrame, and the data is
    // carried within a YUVImage.
    YCBCR420P10 = 6,

    // sRGB, interleaved, each component is a uint16.
    SRGB48 = 7,

    // sRGBA, interleaved, each component is a uint16.
    SRGBA64 = 8,

    // One float per pixel.
    VEC32F1 = 9,

    // Two floats per pixel.
    VEC32F2 = 12,

    // LAB, interleaved: one byte for L, then one byte for a, then one
    // byte for b for each pixel.
    LAB8 = 10,

    // sBGRA, interleaved: one byte for B, one byte for G, one byte for R,
    // one byte for alpha or unused. This is the N32 format for Skia.
    SBGRA = 11,

    // BGR, interleaved: one byte for B, one byte for G, one byte for R
    SBGR = 13,
};

class ImageFrame
{
public:
    using Deleter = std::function<void(uint8_t*)>;

    class PixelDataDeleter
    {
    public:
        static const Deleter kArrayDelete;
        static const Deleter kFree;
        static const Deleter kAlignedFree;
        static const Deleter kNone;
    };

    // Use a default alignment boundary of 16 because Intel SSE2 instructions may
    // incur performance penalty when accessing data not aligned on a 16-byte
    // boundary. FFmpeg requires at least this level of alignment.
    static constexpr uint32_t kDefaultAlignmentBoundary = 16;

    // If the pixel data of an ImageFrame will be passed to an OpenGL function
    // such as glTexImage2D() or glReadPixels(), use a four-byte alignment
    // boundary because that is the initial value of the OpenGL GL_PACK_ALIGNMENT
    // and GL_UNPACK_ALIGNMENT parameters.
    static const uint32_t kGlDefaultAlignmentBoundary = 4;

public:
    // Creates an empty ImageFrame. It will need to be initialized by some other means.
    ImageFrame();
    ImageFrame(ImageFrame&& moveFrom);
    ImageFrame& operator=(ImageFrame&& moveFrom);
    ImageFrame(const ImageFrame&) = delete;
    ImageFrame& operator=(const ImageFrame&) = delete;

    // Allocate a frame of the appropriate size. Does not zero it out.
    // Each row will be aligned to `alignmentBoundary`. `alignmentBoundary`
    // must be a power of 2 (the number 1 is valid, and means the data will
    // be stored contiguously).
    ImageFrame(ImageFormat format, int width, int height, uint32_t alignmentBoundary);
    // Same as above, but use kDefaultAlignmentBoundary for `alignmentBoundary`.
    ImageFrame(ImageFormat format, int width, int height);
    ImageFrame(ImageFormat format, int width, int height, int widthStep, //
               uint8_t* pixelData, Deleter deleter = std::default_delete<uint8_t[]>());

    // Relinquishes ownership of the pixel data.  Notice that the unique_ptr
    // uses a non-standard deleter.
    std::unique_ptr<uint8_t[], ImageFrame::Deleter> release();
    void reset(ImageFormat format, int width, int height, uint32_t alignmentBoundary);
    void adoptPixelData(ImageFormat format, int width, int height, int widthStep, //
                        uint8_t* pixelData, ImageFrame::Deleter deleter);

    // Set the entire frame allocation to zero, including alignment
    // padding areas.
    void setToZero();
    // Returns true if the data is stored contiguously (without any
    // alignment padding areas).
    bool isContiguous() const;
    // Returns true if each row of the data is aligned to
    // alignment_boundary.  If IsAligned(16) is true then so are
    // IsAligned(8), IsAligned(4), IsAligned(2), and IsAligned(1).
    // alignment_boundary must be 1 or a power of 2.
    bool isAligned(uint32_t alignmentBoundary) const;

    void copyFrom(const ImageFrame& imageFrame, uint32_t alignmentBoundary);
    void copyPixelData(ImageFormat format, int width, int height, const uint8_t* pixelData, uint32_t alignmentBoundary);
    void copyPixelData(ImageFormat format, int width, int height, int withStep, const uint8_t* pixelData,
                       uint32_t alignmentBoundary);

    void copyToBuffer(uint8_t* buffer, int bufferSize) const;
    void copyToBuffer(uint16_t* buffer, int bufferSize) const;
    void copyToBuffer(float* buffer, int bufferSize) const;

    static int numberOfChannelsForFormat(ImageFormat format);
    static int channelSizeForFormat(ImageFormat format);
    static int byteDepthForFormat(ImageFormat format);

private:
    void internalCopyFrom(int width, int height, int widthStep, int channelSize, const uint8_t* pixelData);
    void internalCopyToBuffer(int widthStep, char* buffer) const;

public:
    bool isEmpty() const { return pixelData_ == nullptr; }

    ImageFormat format() const { return format_; }
    int width() const { return width_; }
    int height() const { return height_; }
    int channels() const;
    int channelSize() const;
    int byteDepth() const;
    int step() const { return widthStep_; }

    uint8_t* pixelData() { return pixelData_.get(); }
    const uint8_t* pixelData() const { return pixelData_.get(); }
    int pixelDataSize() const { return height() * step(); }

private:
    ImageFormat format_;
    int width_;
    int height_;
    int widthStep_;
    std::unique_ptr<uint8_t[], Deleter> pixelData_;
};

std::ostream& operator<<(std::ostream& os, const ImageFrame& obj);
} // namespace gawrs_face
