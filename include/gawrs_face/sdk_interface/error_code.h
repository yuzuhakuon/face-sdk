#pragma once

#define ERR_BASE 0x00001

enum GawrsFaceErrorCode
{
    GFE_OK = 0x00000,             ///< No error
    GFE_UNKNOWN = (ERR_BASE + 0), ///< Unknown error
};