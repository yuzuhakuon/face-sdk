#pragma once
#include <memory>
#include <shared_mutex>
#include <spdlog/spdlog.h>

#include "gawrs_face/sdk_interface/error_code.h"
#include "gawrs_face/sdk_interface/sdk_interface.h"
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
    RotationModel rotation{RotationModel::GF_ROTATE_0};
};

class FaceEngine
{
public:
    FaceEngine();

    GawrsFaceErrorCode createEngine(const FaceEngineConfig& config);
    void destroyEngine();

    GawrsFaceErrorCode detectFace(unsigned char* idata, int width, int height, int format,
                                  std::vector<Detection>& detections);

    GawrsFaceErrorCode extractFaceFeature(unsigned char* idata, int width, int height, int format, const Detection& det,
                                          FaceFeaturePacked& packed);

    static float compareFaceFeature(const FaceFeaturePacked& packed1, const FaceFeaturePacked& packed2);

private:
    GawrsFaceErrorCode imageCheck(unsigned char* idata, int width, int height, int format);

private:
    bool hasInitialized_{false};
    FaceEngineConfig config_;

    std::shared_ptr<SCRFD> faceDetector_;
    std::shared_ptr<MobileFaceNet> faceExtractor_;
    std::shared_ptr<spdlog::logger> logger_;

    std::shared_mutex mut_;
};

inline int mapToOrientation(RotationModel model);

inline int toNCNN_RGB24(int format);
} // namespace gawrs_face