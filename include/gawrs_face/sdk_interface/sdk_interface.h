#pragma once
#include "sdk_types.h"

#pragma region platform detection and definitions
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define GF_WIN32
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define GF_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#define GF_MAC
#else
#error "Unsupported platform"
#endif

#if defined(GF_WIN32)
#ifdef GF_API_EXPORTS
#define GF_API extern "C" __declspec(dllexport)
#else
#define GF_API extern "C" __declspec(dllimport)
#endif

#elif defined(GF_LINUX)
#define GF_API extern "C"

#elif defined(GF_MAC)
#define GF_API extern "C"

#endif
#pragma endregion

#define _Inout_ /** in and out parameter */
#define _In_    /** in parameter */

/**
 * @brief Initialize a SDK engine
 *
 * @param handle Handle of the engine
 * @param detectFaceScale Scale of the detection
 * @param detectFaceMaxNum Max number of the detection
 * @param combineMask Combine mask of models
 * @param probThreshold Probability threshold
 * @param nmsThreshold NMS threshold
 * @param rotation Rotation of the image
 * @return SDKRet
 */
GF_API GawrsRet initEngine(_Inout_ GawrsHandle* handle, _In_ int detectFaceScale, _In_ int detectFaceMaxNum,
                           _In_ int combineMask, _In_ float probThreshold, _In_ float nmsThreshold, _In_ int rotation);

/**
 * @brief Detect faces in an image
 *
 * @param handle the SDK handle
 * @param img the image
 * @param detectedFaces the detected faces
 * @return SDKRet the error code, 0 means success
 */
GF_API GawrsRet detectFaces(_In_ GawrsHandle handle, _In_ LP_ImageData img, _Inout_ LP_GawrsMultiFaceInfo detectedFaces);

/**
 * @brief Extract face feature from an image
 *
 * @param handle the SDK handle
 * @param img the image
 * @param faceInfo the face information
 * @param feature the face feature
 * @return SDKRet the error code, 0 means success
 */
GF_API GawrsRet extractFaceFeature(_In_ GawrsHandle handle, _In_ LP_ImageData img, _In_ LP_GawrsFaceInfo faceInfo,
                            _Inout_ LP_GawrsFaceFeature feature);

/**
 * Compares two face features and returns the confidence level.
 *
 * @param handle the SDK handle
 * @param feature1 the first face feature to compare
 * @param feature2 the second face feature to compare
 * @param confidenceLevel a pointer to store the confidence level
 *
 * @return the error code, 0 means success
 */
GF_API GawrsRet faceFeatureCompare(_In_ GawrsHandle handle, _In_ LP_GawrsFaceFeature feature1,
                            _In_ LP_GawrsFaceFeature feature2, _Inout_ float* confidenceLevel);

/**
 * @brief Release the SDK engine
 *
 * @param handle the SDK handle
 */
GF_API void releaseEngine(_In_ GawrsHandle handle);

GF_API void gc_LP_SDKMultiFaceInfo(GawrsMultiFaceInfo* multiFaceInfo);
GF_API void gc_LP_SDKFaceFeature(GawrsFaceFeature* feature);