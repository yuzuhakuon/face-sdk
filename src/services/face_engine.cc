#include <vector>

#include "gawrs_face/common/config.h"
#include "gawrs_face/common/image_frame.h"
#include "gawrs_face/common/logger.h"
#include "gawrs_face/services/detector/scrfd.h"
#include "gawrs_face/services/face_engine.h"
#include "gawrs_face/types/face_feature.h"
#include "gawrs_face/utilities/face_align.h"
#include "gawrs_face/utilities/rotation_helper.h"
#include "gawrs_face/utilities/similarity.h"

namespace gawrs_face
{
FaceEngine::FaceEngine() { logger_ = Logger::instance().logger(); }

GawrsFaceErrorCode FaceEngine::initialize(const FaceEngineConfig& config)
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
    logger_->info("Prob Threshold: {0}", config.probThreshold);
    logger_->info("NMS Threshold: {0}", config.nmsThreshold);
    logger_->info("Face Rotation: {0}", (int)config.rotation);

    return ret;
}

GawrsFaceErrorCode FaceEngine::imageCheck(unsigned char* idata, int width, int height, ImageFormat format)
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

GawrsFaceErrorCode FaceEngine::detectFace(unsigned char* idata, int width, int height, ImageFormat format,
                                          std::vector<Detection>& detections)
{
    logger_->info("Detect image with width: {0}, height: {1}, format: {2}", width, height, static_cast<int>(format));
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
    int ch = ImageFrame::numberOfChannelsForFormat(format);
    std::vector<unsigned char> odata(idata, idata + width * height * ch);
    if (config_.rotation != RotationModel::GF_ROTATE_0)
    {
        logger_->info("Restore image with rotation: {0}", (int)config_.rotation);
        auto restoreRotation = undoRotation(config_.rotation);
        auto orientation = mapToOrientation(restoreRotation);
        if ((restoreRotation == RotationModel::GF_ROTATE_90) | (restoreRotation == RotationModel::GF_ROTATE_270))
        {
            std::swap(w, h);
        }
        ncnn::kanna_rotate_c3(idata, width, height, odata.data(), w, h, orientation);
    }

    // Convert the input image to the appropriate format
    auto ncnnFormat = toNCNN_RGB24(format);
    ncnn::Mat inImage = ncnn::Mat::from_pixels(odata.data(), ncnnFormat, w, h);
    auto result = faceDetector_->doInference(inImage, config_.probThreshold, config_.nmsThreshold);
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

        if (config_.rotation == RotationModel::GF_ROTATE_0)
        {
            detections.push_back(det);
        }
        else
        {
            detections.push_back(rotateDetectionWithRelative(det, config_.rotation, width, height));
        }
        ++count;
    }
    logger_->info("Accepted {0} faces", detections.size());
#pragma endregion

    return ret;
}

GawrsFaceErrorCode FaceEngine::extractFaceFeature(unsigned char* idata, int width, int height, ImageFormat format,
                                                  const Detection& det, FaceFeaturePacked& packed)
{
    auto ret = GawrsFaceErrorCode::GFE_OK;
    std::shared_lock lk(mut_);
    if (!hasInitialized_)
    {
        ret = GawrsFaceErrorCode::GFE_ENGINE_NOT_INIT;
        return ret;
    }
    if (!faceExtractor_)
    {
        ret = GawrsFaceErrorCode::GFE_RECOGNIZER_NOT_INIT;
        return ret;
    }
    ret = imageCheck(idata, width, height, format);
    if (ret != GFE_OK)
    {
        return ret;
    }

    int w = width;
    int h = height;
    int ch = ImageFrame::numberOfChannelsForFormat(format);
    std::vector<unsigned char> odata(idata, idata + width * height * ch);
    if (config_.rotation != RotationModel::GF_ROTATE_0)
    {
        logger_->info("Restore image with rotation: {0}", (int)config_.rotation);
        auto restoreRotation = undoRotation(config_.rotation);
        auto orientation = mapToOrientation(restoreRotation);
        if ((restoreRotation == RotationModel::GF_ROTATE_90) | (restoreRotation == RotationModel::GF_ROTATE_270))
        {
            std::swap(w, h);
        }
        ncnn::kanna_rotate_c3(idata, width, height, odata.data(), w, h, orientation);
    }

    // Convert the input image to the appropriate format
    auto rotatedDet = undoRotateDetectionWithRelative(det, config_.rotation, width, height);
    auto ncnnFormat = toNCNN_RGB24(format);
    ncnn::Mat inImage = ncnn::Mat::from_pixels(odata.data(), ncnnFormat, w, h);
    auto cropped = normCrop(inImage, rotatedDet, kAlignedFaceSize, kAlignedFaceSize);
    auto feature = faceExtractor_->doInference(cropped);

    FeatureVersion version{
        .major = kFeatureVersionMajor,
        .minor = kFeatureVersionMinor,
        .patch = kFeatureVersionPatch,
    };
    packed.version = version;
    memcpy(packed.feature, feature.data(), kFeatureSize * sizeof(float));

    return ret;
}

float FaceEngine::compareFaceFeature(const FaceFeaturePacked& packed1, const FaceFeaturePacked& packed2)
{
    auto cosine = cosineSimilarity(packed1.feature, packed2.feature, kFeatureSize);
    return cosine;
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

int toNCNN_RGB24(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::SRGBA:
            return ncnn::Mat::PIXEL_RGBA2RGB;
        case ImageFormat::GRAY8:
            return ncnn::Mat::PIXEL_GRAY2RGB;
        case ImageFormat::SBGRA:
            return ncnn::Mat::PIXEL_BGRA2RGB;
        case ImageFormat::SBGR:
            return ncnn::Mat::PIXEL_BGR2RGB;
        default:
            return ncnn::Mat::PIXEL_RGB;
    }
}
} // namespace gawrs_face