#pragma once

#define ERR_BASE 0x00001
/** SDK base error code */
#define ERR_SDK_BASE 0x01000
/** FaceEngine base error code */
#define ERR_ENGINE_ERROR_BASE 0x02000

enum GawrsFaceErrorCode
{
    GFE_OK = 0x00000,             ///< No error
    GFE_UNKNOWN = (ERR_BASE + 0), ///< Unknown error

    GFE_ENGINE_INIT_FAIL = (ERR_ENGINE_ERROR_BASE + 1),     ///< Engine init error
    GFE_DETECTOR_INIT_FAIL = (ERR_ENGINE_ERROR_BASE + 2),   ///< Detector init error
    GFE_RECOGNIZER_INIT_FAIL = (ERR_ENGINE_ERROR_BASE + 3), ///< Extractor init error
    GFE_UNSUPPORTED_PROPERTY = (ERR_ENGINE_ERROR_BASE + 4), ///< Unsupported property
    GFE_ENGINE_NOT_INIT = (ERR_ENGINE_ERROR_BASE + 5),      ///< Engine not init
    GFE_DETECTOR_NOT_INIT = (ERR_ENGINE_ERROR_BASE + 6),    ///< Detector not init
    GFE_RECOGNIZER_NOT_INIT = (ERR_ENGINE_ERROR_BASE + 7),  ///< Extractor not init
    GFE_INVALID_IMAGE = (ERR_ENGINE_ERROR_BASE + 8),        ///< Invalid image
    GFE_IMAGE_TOO_LARGE = (ERR_ENGINE_ERROR_BASE + 9),      ///< Image too large
    GFE_IMAGE_TOO_SMALL = (ERR_ENGINE_ERROR_BASE + 10),     ///< Image too small

    GFE_MISMATCHFEATURE_LEVEL = (ERR_ENGINE_ERROR_BASE + 11), ///< Mismatch feature level
    GFE_MISMATCHFEATURE_SIZE = (ERR_ENGINE_ERROR_BASE + 12),  ///< Mismatch feature size

};