#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include "com_gawrs_face_FaceKit.h"
#include "gawrs_face/common/config.h"
#include "gawrs_face/common/logger.h"
#include "gawrs_face/common/version.h"
#include "gawrs_face/sdk_interface/error_code.h"
#include "gawrs_face/services/face_engine.h"
#include "jni_face_feature.h"
#include "jni_face_info.h"
#include "jni_face_kit.h"
#include "jni_md.h"
#include "jni_similar.h"
#include "jni_utils.h"
#include "jni_version_info.h"

using namespace gawrs_face;

jint Java_com_gawrs_face_FaceKit_initFaceEngine(JNIEnv* env, jobject self, jint detectFaceScaleVal,
                                                jint detectFaceMaxNum, jint combinedMask, jfloat probThreshold,
                                                jfloat nmsThreshold, jint rotation)
{
    auto logger = Logger::instance().logger();
    auto ret = GawrsFaceErrorCode::GFE_OK;

    auto faceKitManaged = std::make_shared<FaceKitManaged>(env);
    auto handle = faceKitManaged->handle(env, self);
    if (handle != 0L)
    {
        logger->warn("FaceKit has been initialized.");
        return ret;
    }

    FaceEngineConfig config;
    config.detectFaceScaleVal = detectFaceScaleVal;
    config.detectFaceMaxNum = detectFaceMaxNum;
    config.combinedMask = combinedMask;
    config.probThreshold = probThreshold;
    config.nmsThreshold = nmsThreshold;
    config.rotation = static_cast<RotationModel>(rotation);

    auto engine = new FaceEngine();
    ret = engine->initialize(config);
    if (ret == GawrsFaceErrorCode::GFE_OK)
    {
        logger->info("FaceEngine init success.");
        faceKitManaged->setHandle(env, self, reinterpret_cast<jlong>(engine));
    }
    else
    {
        logger->error("FaceEngine init failed with {}.", (int)ret);
        delete engine;
    }

    return ret;
}

jint Java_com_gawrs_face_FaceKit_detectFaces(JNIEnv* env, jobject self, jlong handle, jbyteArray buffer, jint width,
                                             jint height, jint format, jobject faceInfos)
{
    if (handle == 0L)
    {
        return GawrsFaceErrorCode::GFE_ENGINE_NOT_INIT;
    }
    auto ret = GawrsFaceErrorCode::GFE_OK;
    auto engine = reinterpret_cast<FaceEngine*>(handle);

    std::vector<Detection> detections;
    auto imgData = byteArray2Vector(env, buffer);
    ret = engine->detectFace(imgData.data(), width, height, static_cast<ImageFormat>(format), detections);
    if (ret == GFE_OK)
    {
        const auto [faceInfoManaged, faceRectManaged, facePointManaged, arrayListManaged] = createFaceInfoManaged(env);
        auto faceInfosRef = JArrayListRef(arrayListManaged, env, faceInfos);
        for (const auto& det : detections)
        {
            auto relativeBox = std::get_if<RelativeBoundingBox>(&det.boundingBox);
            auto keypoints = std::get_if<RelativeKeypoints>(&det.keypoints);
            if (relativeBox != nullptr && keypoints != nullptr)
            {
                auto faceInfo = JFaceInfoRef(env, faceInfoManaged, faceRectManaged, facePointManaged, arrayListManaged);
                faceInfo.setfaceId(-1);
                faceInfo.setRect(relativeBox->xmin, relativeBox->ymin, relativeBox->xmin + relativeBox->width,
                                 relativeBox->ymin + relativeBox->height);
                for (int k = 0; k < keypoints->size(); ++k)
                {
                    auto& p = (*keypoints)[k];
                    faceInfo.addPoint(p.x(), p.y());
                }
                faceInfosRef.add(faceInfo.self());
            }
        }
    }

    return ret;
}

jint Java_com_gawrs_face_FaceKit_extractFaceFeature(JNIEnv* env, jobject self, jlong handle, jbyteArray buffer,
                                                    jint width, jint height, jint format, jobject faceInfo,
                                                    jobject faceFeature)
{
    if (handle == 0L)
    {
        return GawrsFaceErrorCode::GFE_ENGINE_NOT_INIT;
    }
    auto ret = GawrsFaceErrorCode::GFE_OK;
    auto engine = reinterpret_cast<FaceEngine*>(handle);

    const auto [faceInfoManaged, faceRectManaged, facePointManaged, arrayListManaged] = createFaceInfoManaged(env);
    auto faceInfoRef =
        JFaceInfoRef(env, faceInfoManaged, faceRectManaged, facePointManaged, arrayListManaged, faceInfo);
    auto box = faceInfoRef.faceRect();
    auto keypoints = faceInfoRef.keypoints();
    Detection det{.label = 0, .score = 0.f, .boundingBox = box, .keypoints = keypoints};

    FaceFeaturePacked packed;
    auto imgData = byteArray2Vector(env, buffer);
    ret = engine->extractFaceFeature(imgData.data(), width, height, static_cast<ImageFormat>(format), det, packed);
    if (ret == GFE_OK)
    {
        auto faceFeatureManaged = std::make_shared<FaceFeatureManaged>(env);
        auto faceFeatureRef = JFaceFeatureRef(env, faceFeatureManaged, faceFeature);
        faceFeatureRef.setData((jbyte*)packed._data, kPackedFeatureByteSize);
    }

    return ret;
}

jint Java_com_gawrs_face_FaceKit_compareFaceFeature(JNIEnv* env, jobject self, jlong handle, jobject faceFeature1,
                                                    jobject faceFeature2, jobject faceSimilar)
{
    auto logger = Logger::instance().logger();
    auto ret = GawrsFaceErrorCode::GFE_OK;

    auto faceFeatureManaged = std::make_shared<FaceFeatureManaged>(env);
    auto faceFeatureRef1 = JFaceFeatureRef(env, faceFeatureManaged, faceFeature1);
    auto faceFeatureRef2 = JFaceFeatureRef(env, faceFeatureManaged, faceFeature2);
    if ((faceFeatureRef1.size() != faceFeatureRef1.size()) || (faceFeatureRef1.size() < kPackedFeatureByteSize))
    {
        ret = GFE_MISMATCHFEATURE_LEVEL;
    }

    if (ret == GFE_OK)
    {
        FaceFeaturePacked feature1, feature2;
        faceFeatureRef1.copyTo((jbyte*)feature1._data, kPackedFeatureByteSize);
        faceFeatureRef2.copyTo((jbyte*)feature2._data, kPackedFeatureByteSize);
        auto sim = FaceEngine::compareFaceFeature(feature1, feature2);

        auto managed = std::make_shared<FaceSimilarManaged>(env);
        auto faceSimilarRef = JFaceSimilarRef(env, managed, faceSimilar);
        faceSimilarRef.setScore(sim);
    }

    return ret;
}

void Java_com_gawrs_face_FaceKit_getVersionInfo(JNIEnv* env, jobject self, jobject versionInfo)
{
    auto versionInfoManaged = std::make_shared<VersionInfoManaged>(env);

    auto versionInfoRef = JVersionInfoRef(env, versionInfoManaged, versionInfo);
    versionInfoRef.setVersion(coreVersion);
    versionInfoRef.setBuildDate(buildDate);
    versionInfoRef.setCopyRight(copyRight);
}
