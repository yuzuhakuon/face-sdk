#include "gawrs_face/sdk_interface/sdk_interface.h"
#include "gawrs_face/common/image_frame.h"
#include "gawrs_face/sdk_interface/error_code.h"
#include "gawrs_face/services/face_engine.h"
#include "gawrs_face/types/face_feature.h"

using namespace gawrs_face;

GawrsRet initEngine(GawrsHandle* handle, int detectFaceScale, int detectFaceMaxNum, int combineMask,
                    float probThreshold, float nmsThreshold, int rotation)
{
    auto ret = GawrsFaceErrorCode::GFE_OK;
    auto engine = new FaceEngine();
    FaceEngineConfig config{
        .detectFaceScaleVal = detectFaceScale,
        .detectFaceMaxNum = detectFaceMaxNum,
        .combinedMask = combineMask,
        .probThreshold = probThreshold,
        .nmsThreshold = nmsThreshold,
        .rotation = static_cast<RotationModel>(rotation),
    };

    ret = engine->initialize(config);
    if (ret != GawrsFaceErrorCode::GFE_OK)
    {
        delete engine;
    }
    else
    {
        *handle = (void*)engine;
    }

    return ret;
}

GawrsRet detectFaces(GawrsHandle handle, LP_ImageData img, LP_GawrsMultiFaceInfo detectedFaces)
{
    auto engine = (FaceEngine*)(handle);
    if (!engine)
    {
        return GawrsFaceErrorCode::GFE_ENGINE_NOT_INIT;
    }

    auto ret = GawrsFaceErrorCode::GFE_OK;
    std::vector<Detection> output;
    ImageFormat format = static_cast<ImageFormat>(img->format);
    ret = engine->detectFace(img->data, img->width, img->height, format, output);
    if (ret == GFE_OK)
    {
        detectedFaces->faceNum = output.size();
        if (output.size() > 0)
        {
            detectedFaces->faceRect = new GawrsRect[output.size()];
            detectedFaces->facePoints = new GawrsPoint[output.size() * 5];
            detectedFaces->faceScore = new float[output.size()];
            for (int i = 0; i < output.size(); ++i)
            {
                const auto& det = output[i];
                auto& bbox = std::get<RelativeBoundingBox>(det.boundingBox);
                auto& kps = std::get<RelativeKeypoints>(det.keypoints);

                detectedFaces->faceRect[i].left = bbox.xmin;
                detectedFaces->faceRect[i].top = bbox.ymin;
                detectedFaces->faceRect[i].right = bbox.xmin + bbox.width;
                detectedFaces->faceRect[i].bottom = bbox.ymin + bbox.height;
                detectedFaces->faceScore[i] = det.score;
                for (int j = 0; j < 5; ++j)
                {
                    detectedFaces->facePoints[i * 5 + j].x = kps[j].x();
                    detectedFaces->facePoints[i * 5 + j].y = kps[j].y();
                }
            }
        }
    }

    return ret;
}

GawrsRet extractFaceFeature(GawrsHandle handle, LP_ImageData img, LP_GawrsFaceInfo faceInfo,
                            LP_GawrsFaceFeature feature)
{
    auto engine = (FaceEngine*)(handle);
    if (!engine)
    {
        return GawrsFaceErrorCode::GFE_ENGINE_NOT_INIT;
    }
    auto ret = GawrsFaceErrorCode::GFE_OK;

    RelativeBoundingBox bbox;
    bbox.xmin = faceInfo->faceRect.left;
    bbox.ymin = faceInfo->faceRect.top;
    bbox.width = faceInfo->faceRect.right - faceInfo->faceRect.left;
    bbox.height = faceInfo->faceRect.bottom - faceInfo->faceRect.top;

    RelativeKeypoints keypoints;
    for (int i = 0; i < 5; ++i)
    {
        float x = faceInfo->landmark[i].x;
        float y = faceInfo->landmark[i].y;
        keypoints.emplace_back(x, y);
    }

    Detection det{.label = 0, .score = faceInfo->faceScore, .boundingBox = bbox, .keypoints = keypoints};

    FaceFeaturePacked packed;
    ImageFormat format = static_cast<ImageFormat>(img->format);
    ret = engine->extractFaceFeature(img->data, img->width, img->height, format, det, packed);
    if (ret == GawrsFaceErrorCode::GFE_OK)
    {
        constexpr auto featureSize = sizeof(packed);
        feature->feature = new CUInt8[featureSize];
        feature->featureSize = featureSize;
        memcpy(feature->feature, &packed, featureSize);
    }

    return ret;
}

GawrsRet faceFeatureCompare(GawrsHandle handle, LP_GawrsFaceFeature feature1, LP_GawrsFaceFeature feature2,
                            float* confidenceLevel)
{
    auto ret = GawrsFaceErrorCode::GFE_OK;
    if (feature1->featureSize != feature2->featureSize)
    {
        ret = GawrsFaceErrorCode::GFE_MISMATCHFEATURE_SIZE;
    }
    if (feature1->featureSize != sizeof(FaceFeaturePacked))
    {
        ret = GawrsFaceErrorCode::GFE_MISMATCHFEATURE_SIZE;
    }

    if (ret == GawrsFaceErrorCode::GFE_OK)
    {
        FaceFeaturePacked packed1, packed2;
        memcpy(&packed1, feature1->feature, feature1->featureSize);
        memcpy(&packed2, feature2->feature, feature2->featureSize);
        if (packed1.version != packed2.version)
        {
            ret = GawrsFaceErrorCode::GFE_MISMATCHFEATURE_LEVEL;
        }
        else
        {
            float cosine = FaceEngine::compareFaceFeature(packed1, packed2);
            *confidenceLevel = cosine;
        }
    }

    return ret;
}

void releaseEngine(GawrsHandle handle)
{
    auto engine = (FaceEngine*)(handle);
    delete engine;
}

void gc_LP_SDKMultiFaceInfo(GawrsMultiFaceInfo* multiFaceInfo)
{
    delete[] multiFaceInfo->faceRect;
    delete[] multiFaceInfo->facePoints;
    delete[] multiFaceInfo->faceScore;
    multiFaceInfo->faceNum = 0;
}
void gc_LP_SDKFaceFeature(GawrsFaceFeature* feature)
{
    delete[] feature->feature;
    feature->feature = nullptr;
    feature->featureSize = 0;
}
