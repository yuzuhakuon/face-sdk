#pragma once

#define LANDMARK_NUM 5

#define GF_UNKNOWN 0,
#define GF_BGR24 = 13,
#define GF_RGB24 = 1,
#define GF_GRAY = 3,
#define GF_BGRA = 11,
#define GF_RGBA = 2,

enum RuntimeModel
{
    GF_NONE = 0,                      // no model loaded
    GF_FACE_DETECTION = 0b00000001,   // scrfd for face detection
    GF_FACE_RECOGNITION = 0b00000010, // mobilefacenet for face recognition
};

enum RotationModel
{
    GF_ROTATE_0 = 1,
    GF_ROTATE_90 = 6,
    GF_ROTATE_180 = 3,
    GF_ROTATE_270 = 8,
    GF_HorizontalFlip = 2,
    GF_VerticalFlip = 4,
    GF_RotationFlip = 5, ///< Unimplemented!!
    GF_FlipRotation = 7, ///< Unimplemented!!
};

typedef signed char CInt8;
typedef unsigned char CUInt8;
typedef signed int CInt32;
typedef unsigned int CUInt32;

typedef void* GawrsHandle;
typedef int GawrsRet;

typedef struct
{
    CUInt32 format;
    CInt32 width;
    CInt32 height;
    CUInt8* data;
} ImageData, *LP_ImageData;

typedef struct
{
    float left;
    float top;
    float right;
    float bottom;
} GawrsRect, *LP_GawrsRect;

typedef struct
{
    float x;
    float y;

} GawrsPoint, *LP_GawrsPoint;

typedef GawrsPoint SDKLandmark[LANDMARK_NUM];

typedef struct
{
    GawrsRect faceRect;
    SDKLandmark landmark;
    float faceScore;
} GawrsFaceInfo, *LP_GawrsFaceInfo;

typedef struct
{
    GawrsRect* faceRect;
    GawrsPoint* facePoints;
    float* faceScore;
    CUInt32 faceNum;
} GawrsMultiFaceInfo, *LP_GawrsMultiFaceInfo;

typedef struct
{
    CUInt8* feature;
    CUInt32 featureSize;
} GawrsFaceFeature, *LP_GawrsFaceFeature;
