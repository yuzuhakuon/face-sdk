#pragma once
#include <cstdint>
#include <iostream>

#include "gawrs_face/common/config.h"

namespace gawrs_face
{
union FeatureVersion
{
    struct
    {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
    };
    uint32_t _data;

    friend std::ostream& operator<<(std::ostream& out, const FeatureVersion& v)
    {
        out << (int)v.major << "." << (int)v.minor << "." << (int)v.patch;
        return out;
    }
};

union FaceFeaturePacked
{
    struct
    {
        float feature[kFeatureSize];
        FeatureVersion version;
    };
    float _data[kPackedFeatureSize];
};
} // namespace gawrs_face