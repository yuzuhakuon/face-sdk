#include "gawrs_face/services/face_engine.h"
#include "gawrs_face/common/config.h"
#include "gawrs_face/common/logger.h"
#include "gawrs_face/sdk_interface/sdk_interface.h"
#include "gawrs_face/services/detector/scrfd.h"

namespace gawrs_face
{
FaceEngine::FaceEngine() { logger_ = Logger::instance().logger(); }

GawrsFaceErrorCode FaceEngine::createEngine(const FaceEngineConfig& config)
{
    logger_->info("Create Face Engine");
    auto ret = GawrsFaceErrorCode::GFE_OK;
    std::lock_guard lk(mut_);
    if (hasInitialized_)
    {
        return ret;
    }

    if (config.combinedMask & RuntimeModel::GF_FACE_DETECTION)
    {
        logger_->info("Loading face detector");
        faceDetector_ = std::make_shared<SCRFD>();
        bool success = faceDetector_->loadModelInMemory();
        if (!success)
        {
            logger_->error("Failed to load face detector");
            faceDetector_.reset();
            ret = GawrsFaceErrorCode::GFE_DETECTOR_INIT_FAIL;
            return ret;
        }
    }
    if (config.combinedMask & RuntimeModel::GF_FACE_RECOGNITION)
    {
        logger_->info("Loading face recognizer");
        faceExtractor_ = std::make_shared<MobileFaceNet>();
        bool success = faceExtractor_->loadModelInMemory();
        if (!success)
        {
            logger_->error("Failed to load face recognizer");
            faceExtractor_.reset();
            ret = GawrsFaceErrorCode::GFE_RECOGNIZER_INIT_FAIL;
            return ret;
        }
    }

    config_ = config;
    hasInitialized_ = true;

    logger_->info("Running models: ");
    logger_->info("Face Detection: {0}",
                  (config.combinedMask & RuntimeModel::GF_FACE_DETECTION) > 0 ? "true" : "false");
    logger_->info("Face Recognition: {0}",
                  (config.combinedMask & RuntimeModel::GF_FACE_RECOGNITION) > 0 ? "true" : "false");
    logger_->info("CreateEngine done");

    logger_->info("Configuration:");
    logger_->info("Combined Mask: {0}", config.combinedMask);
    logger_->info("Detect Face Scale Value: {0}", config.detectFaceScaleVal);
    logger_->info("Detect Face Max Number: {0}", config.detectFaceMaxNum);

    return ret;
}

GawrsFaceErrorCode FaceEngine::imageCheck(unsigned char* idata, int width, int height, int format)
{
    auto ret = GawrsFaceErrorCode::GFE_OK;
    if (idata == nullptr)
    {
        ret = GawrsFaceErrorCode::GFE_UNKNOWN;
        logger_->error("Invalid idata pointer");
    }
    if (width < kAlignedFaceSize || height < kAlignedFaceSize)
    {
        ret = GawrsFaceErrorCode::GFE_IMAGE_TOO_SMALL;
        logger_->error("Invalid image size, width: {0}, height: {1}", width, height);
    }
    return ret;
}

GawrsFaceErrorCode FaceEngine::detectFace(unsigned char* idata, int width, int height, int format,
                                          std::vector<Detection>& detections)
{
    logger_->info("Detect image with width: {0}, height: {1}, format: {2}", width, height, format);
    auto ret = GawrsFaceErrorCode::GFE_OK;
    std::shared_lock lk(mut_);
    if (!hasInitialized_)
    {
        ret = GawrsFaceErrorCode::GFE_ENGINE_NOT_INIT;
        return ret;
    }
    if (!faceDetector_)
    {
        ret = GawrsFaceErrorCode::GFE_DETECTOR_NOT_INIT;
        return ret;
    }
    ret = imageCheck(idata, width, height, format);
    if (ret != GFE_OK)
    {
        return ret;
    }

    int w = width;
    int h = height;
    if (config_.rotation != RotationModel::GF_ROTATE_0)
    {
        logger_->info("Rotate image with rotation: {0}", (int)config_.rotation);
        auto orientation = mapToOrientation(config_.rotation);
        if ((config_.rotation == RotationModel::GF_ROTATE_90) | (config_.rotation == RotationModel::GF_ROTATE_270))
        {
            std::swap(width, height);
        }
        ncnn::kanna_rotate_c3(idata, w, h, idata, width, height, orientation);
    }

    // Convert the input image to the appropriate format
    auto ncnnFormat = toNCNN_RGB24(format);
    ncnn::Mat inImage = ncnn::Mat::from_pixels(idata, ncnnFormat, width, height);
    auto result = faceDetector_->doInference(inImage);
    logger_->info("Detected {0} faces", result.size());

#pragma region Filter the detections
    int count = 0;
    for (const auto& det : result)
    {
        if (count >= config_.detectFaceMaxNum)
            break;

        const auto bbox = std::get<RelativeBoundingBox>(det.boundingBox);
        if ((bbox.width * config_.detectFaceScaleVal) < width)
            continue;
        detections.push_back(det);
        ++count;
    }
    logger_->info("Accepted {0} faces", detections.size());
#pragma endregion

    return ret;
}

int mapToOrientation(RotationModel model)
{
    switch (model)
    {
        case RotationModel::GF_ROTATE_0:
            return 1;
        case RotationModel::GF_ROTATE_90:
            return 6;
        case RotationModel::GF_ROTATE_180:
            return 3;
        case RotationModel::GF_ROTATE_270:
            return 8;
        case RotationModel::GF_HorizontalFlip:
            return 2;
        case RotationModel::GF_VerticalFlip:
            return 4;
        case RotationModel::GF_RotationFlip:
            return 5;
        case RotationModel::GF_FlipRotation:
            return 7;
        default:
            return 1;
    }
}

inline int toNCNN_RGB24(int format)
{
    if (format == ImageFormat::GF_BGRA)
    {
        return ncnn::Mat::PIXEL_BGRA2RGB;
    }
    else if (format == (int)ImageFormat::GF_RGBA)
    {
        return ncnn::Mat::PIXEL_RGBA2RGB;
    }
    else if (format == (int)ImageFormat::GF_BGR24)
    {
        return ncnn::Mat::PIXEL_BGR2RGB;
    }
    else if (format == (int)ImageFormat::GF_RGB24)
    {
        return ncnn::Mat::PIXEL_RGB;
    }
    else if (format == (int)ImageFormat::GF_GRAY)
    {
        return ncnn::Mat::PIXEL_GRAY2RGB;
    }

    return ncnn::Mat::PIXEL_RGB;
}
} // namespace gawrs_face