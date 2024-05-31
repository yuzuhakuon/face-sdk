#include <algorithm>
#include <cstring>
#include <memory>
#include <sstream>

#include "gawrs_face/common/image_frame.h"
#include "gawrs_face/utilities/aligned_malloc_and_free.h"

namespace gawrs_face
{
const ImageFrame::Deleter ImageFrame::PixelDataDeleter::kArrayDelete = std::default_delete<uint8_t[]>();
const ImageFrame::Deleter ImageFrame::PixelDataDeleter::kFree = free;
const ImageFrame::Deleter ImageFrame::PixelDataDeleter::kAlignedFree = alignedFree;
const ImageFrame::Deleter ImageFrame::PixelDataDeleter::kNone = [](uint8_t* x) {};

ImageFrame::ImageFrame() : format_(ImageFormat::UNKNOWN), width_(0), height_(0), widthStep_(0) {}
ImageFrame::ImageFrame(ImageFormat format, int width, int height)
{ //
    reset(format, width, height, kDefaultAlignmentBoundary);
}
ImageFrame::ImageFrame(ImageFormat format, int width, int height, int widthStep, uint8_t* pixelData, Deleter deleter)
{ //
    adoptPixelData(format, width, height, widthStep, pixelData, deleter);
}

ImageFrame::ImageFrame(ImageFrame&& move_from) { *this = std::move(move_from); }

ImageFrame& ImageFrame::operator=(ImageFrame&& move_from)
{
    pixelData_ = std::move(move_from.pixelData_);
    format_ = move_from.format_;
    width_ = move_from.width_;
    height_ = move_from.height_;
    widthStep_ = move_from.widthStep_;

    move_from.format_ = ImageFormat::UNKNOWN;
    move_from.width_ = 0;
    move_from.height_ = 0;
    move_from.widthStep_ = 0;
    return *this;
}

std::unique_ptr<uint8_t[], ImageFrame::Deleter> ImageFrame::release() { return std::move(pixelData_); }

void ImageFrame::reset(ImageFormat format, int width, int height, uint32_t alignmentBoundary)
{
    format_ = format;
    width_ = width;
    height_ = height;
    widthStep_ = width * channels() * byteDepth();
    if (alignmentBoundary == 1)
    {
        pixelData_ = {new uint8_t[height * widthStep_], PixelDataDeleter::kArrayDelete};
    }
    else
    {
        widthStep_ = ((widthStep_ - 1) | (alignmentBoundary - 1)) + 1;
        pixelData_ = {reinterpret_cast<uint8_t*>(alignedMalloc(height * widthStep_, alignmentBoundary)),
                      PixelDataDeleter::kAlignedFree};
    }
}

// Be sure the `format` is not equal to `ImageFormat::UNKNOWN`, and the `step` is
// equal to `channels() * byteDepth()`
void ImageFrame::adoptPixelData(ImageFormat format, int width, int height, int step, uint8_t* data,
                                ImageFrame::Deleter deleter)
{
    format_ = format;
    width_ = width;
    height_ = height;
    widthStep_ = step;
    pixelData_ = {data, deleter};
}

void ImageFrame::setToZero()
{
    if (pixelData_)
    {
        std::fill_n(pixelData_.get(), widthStep_ * height_, 0);
    }
}

bool ImageFrame::isContiguous() const
{
    if (!pixelData_)
    {
        return false;
    }

    return widthStep_ == width_ * channels() * byteDepth();
}

bool ImageFrame::isAligned(uint32_t alignmentBoundary) const
{
    if (!pixelData_)
    {
        return false;
    }

    if ((reinterpret_cast<uintptr_t>(pixelData_.get()) % alignmentBoundary) != 0)
    {
        return false;
    }

    if ((widthStep_ % alignmentBoundary) != 0)
    {
        return false;
    }

    return true;
}

int ImageFrame::channels() const { return numberOfChannelsForFormat(format()); }
int ImageFrame::channelSize() const { return channelSizeForFormat(format()); }
int ImageFrame::byteDepth() const { return byteDepthForFormat(format()); }

int ImageFrame::numberOfChannelsForFormat(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::SRGB:
            return 3;
        case ImageFormat::SRGBA:
            return 4;
        case ImageFormat::GRAY8:
            return 1;
        case ImageFormat::GRAY16:
            return 1;
        case ImageFormat::SRGB48:
            return 3;
        case ImageFormat::SRGBA64:
            return 4;
        case ImageFormat::VEC32F1:
            return 1;
        case ImageFormat::VEC32F2:
            return 2;
        case ImageFormat::LAB8:
            return 3;
        case ImageFormat::SBGRA:
            return 4;
        case ImageFormat::SBGR:
            return 3;
        default:
            return 0;
    }
}

int ImageFrame::channelSizeForFormat(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::SRGB:
            return sizeof(uint8_t);
        case ImageFormat::SRGBA:
            return sizeof(uint8_t);
        case ImageFormat::GRAY8:
            return sizeof(uint8_t);
        case ImageFormat::GRAY16:
            return sizeof(uint16_t);
        case ImageFormat::SRGB48:
            return sizeof(uint16_t);
        case ImageFormat::SRGBA64:
            return sizeof(uint16_t);
        case ImageFormat::VEC32F1:
            return sizeof(float);
        case ImageFormat::VEC32F2:
            return sizeof(float);
        case ImageFormat::LAB8:
            return sizeof(uint8_t);
        case ImageFormat::SBGRA:
            return sizeof(uint8_t);
        case ImageFormat::SBGR:
            return sizeof(uint8_t);
        default:
            return 0;
    }
};

int ImageFrame::byteDepthForFormat(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::SRGB:
            return 1;
        case ImageFormat::SRGBA:
            return 1;
        case ImageFormat::GRAY8:
            return 1;
        case ImageFormat::GRAY16:
            return 2;
        case ImageFormat::SRGB48:
            return 2;
        case ImageFormat::SRGBA64:
            return 2;
        case ImageFormat::VEC32F1:
            return 4;
        case ImageFormat::VEC32F2:
            return 4;
        case ImageFormat::LAB8:
            return 1;
        case ImageFormat::SBGRA:
            return 1;
        case ImageFormat::SBGR:
            return 1;
        default:
            return 0;
    }
};

void ImageFrame::internalCopyFrom(int width, int height, int widthStep, int channelSize, const uint8_t* pixelData)
{
    const int rowBytes = channelSize * channels() * width;
    if (widthStep == 0)
    {
        widthStep = channelSize * channels() * width;
    }

    const char* src = reinterpret_cast<const char*>(pixelData);
    char* dst = reinterpret_cast<char*>(pixelData_.get());

    if (widthStep == rowBytes && widthStep_ == rowBytes)
    {
        std::memcpy(dst, src, height_ * rowBytes);
    }
    else
    {
        for (int i = height_; i > 0; --i)
        {
            memcpy(dst, src, rowBytes);
            src += widthStep;
            dst += widthStep_;
        }
    }
}

void ImageFrame::internalCopyToBuffer(int widthStep, char* buffer) const
{
    const int rowBytes = channelSize() * channels() * width_;
    if (widthStep == 0)
    {
        widthStep = channelSize() * channels() * width_;
    }

    const char* src = reinterpret_cast<const char*>(pixelData_.get());
    char* dst = buffer;
    if (widthStep == rowBytes && widthStep_ == rowBytes)
    {
        std::memcpy(dst, src, height_ * rowBytes);
    }
    else
    {
        for (int i = height_; i > 0; --i)
        {
            memcpy(dst, src, rowBytes);
            src += widthStep_;
            dst += widthStep;
        }
    }
}

void ImageFrame::copyFrom(const ImageFrame& imageFrame, uint32_t alignmentBoundary)
{
    reset(imageFrame.format(), imageFrame.width(), imageFrame.height(), alignmentBoundary);
    internalCopyFrom(imageFrame.width(), imageFrame.height(), imageFrame.step(), imageFrame.channelSize(),
                     imageFrame.pixelData());
}
void ImageFrame::copyPixelData(ImageFormat format, int width, int height, const uint8_t* pixelData,
                               uint32_t alignmentBoundary)
{
    copyPixelData(format, width, height, 0, pixelData, alignmentBoundary);
}
void ImageFrame::copyPixelData(ImageFormat format, int width, int height, int withStep, const uint8_t* pixelData,
                               uint32_t alignmentBoundary)
{
    reset(format, width, height, alignmentBoundary);
    internalCopyFrom(width, height, withStep, channelSizeForFormat(format), pixelData);
}

void ImageFrame::copyToBuffer(uint8_t* buffer, int bufferSize) const
{
    // dataSize <= bufferSize
    const int dataSize = width_ * height_ * channels();
    if (isContiguous())
    {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(pixelData_.get());
        std::copy_n(src, dataSize, buffer);
    }
    else
    {
        internalCopyToBuffer(0, reinterpret_cast<char*>(buffer));
    }
}

void ImageFrame::copyToBuffer(uint16_t* buffer, int bufferSize) const
{
    // dataSize <= bufferSize
    // byteDepth == 2
    const int dataSize = width_ * height_ * channels();
    if (isContiguous())
    {
        const uint16_t* src = reinterpret_cast<const uint16_t*>(pixelData_.get());
        std::copy_n(src, dataSize, buffer);
    }
    else
    {
        internalCopyToBuffer(0, reinterpret_cast<char*>(buffer));
    }
}

void ImageFrame::copyToBuffer(float* buffer, int bufferSize) const
{
    // dataSize <= bufferSize
    // byteDepth == 4
    const int dataSize = width_ * height_ * channels();
    if (isContiguous())
    {
        const float* src = reinterpret_cast<const float*>(pixelData_.get());
        std::copy_n(src, dataSize, buffer);
    }
    else
    {
        internalCopyToBuffer(0, reinterpret_cast<char*>(buffer));
    }
}

std::ostream& operator<<(std::ostream& os, const ImageFrame& obj)
{
    return os << "[" << obj.width() << ", " << obj.height() << ", " << obj.channels() << "]";
}
} // namespace gawrs_face
