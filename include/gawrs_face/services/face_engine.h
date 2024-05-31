#pragma once
#include <memory>
#include <shared_mutex>
#include <spdlog/spdlog.h>

#include "gawrs_face/common/image_frame.h"
#include "gawrs_face/sdk_interface/error_code.h"
#include "gawrs_face/sdk_interface/sdk_types.h"
#include "gawrs_face/services/detector/scrfd.h"
#include "gawrs_face/services/extractor/mobilefacenet.h"
#include "gawrs_face/types/face_feature.h"

namespace gawrs_face
{
struct FaceEngineConfig
{
    int detectFaceScaleVal{11};
    int detectFaceMaxNum{20};
    int combinedMask{0};
    float probThreshold{0.6f};
    float nmsThreshold{0.45f};
    RotationModel rotation{RotationModel::GF_ROTATE_0};

    void setSupportFaceDetect(bool support)
    {
        if (support)
        {
            combinedMask |= RuntimeModel::GF_FACE_DETECTION;
        }
        else
        {
            combinedMask &= ~RuntimeModel::GF_FACE_DETECTION;
        }
    }

    bool isSupportFaceDetect() const { return (combinedMask & RuntimeModel::GF_FACE_DETECTION) > 0; }

    void setSupportFaceRecognize(bool support)
    {
        if (support)
        {
            combinedMask |= RuntimeModel::GF_FACE_RECOGNITION;
        }
        else
        {
            combinedMask &= ~RuntimeModel::GF_FACE_RECOGNITION;
        }
    }

    bool isSupportFaceRecognize() const { return (combinedMask & RuntimeModel::GF_FACE_RECOGNITION) > 0; }
};

class FaceEngine
{
public:
    FaceEngine();

    GawrsFaceErrorCode initialize(const FaceEngineConfig& config);

    GawrsFaceErrorCode detectFace(unsigned char* idata, int width, int height, ImageFormat format,
                                  std::vector<Detection>& detections);

    GawrsFaceErrorCode extractFaceFeature(unsigned char* idata, int width, int height, ImageFormat format,
                                          const Detection& det, FaceFeaturePacked& packed);

    static float compareFaceFeature(const FaceFeaturePacked& packed1, const FaceFeaturePacked& packed2);

public:
    FaceEngineConfig config() { return config_; }

    const FaceEngineConfig& config() const { return config_; }

    void setDetectFaceScaleVal(int val) { config_.detectFaceScaleVal = val; }

    void setDetectFaceMaxNum(int val) { config_.detectFaceMaxNum = val; }

    void setProbThreshold(float val) { config_.probThreshold = val; }

    void setNMSThreshold(float val) { config_.nmsThreshold = val; }

    void setRotation(RotationModel val) { config_.rotation = val; }

private:
    GawrsFaceErrorCode imageCheck(unsigned char* idata, int width, int height, ImageFormat format);

private:
    bool hasInitialized_{false};
    FaceEngineConfig config_;

    std::shared_ptr<SCRFD> faceDetector_;
    std::shared_ptr<MobileFaceNet> faceExtractor_;
    std::shared_ptr<spdlog::logger> logger_;

    std::shared_mutex mut_;
};

inline int mapToOrientation(RotationModel model);

inline int toNCNN_RGB24(ImageFormat format);
} // namespace gawrs_face