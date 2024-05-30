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

    bool operator==(const FeatureVersion& v) const { return major == v.major && minor == v.minor && patch == v.patch; }
    bool operator!=(const FeatureVersion& v) const { return !(*this == v); }
    bool operator<(const FeatureVersion& v) const
    {
        if (major < v.major)
            return true;
        if (major > v.major)
            return false;
        if (minor < v.minor)
            return true;
        if (minor > v.minor)
            return false;
        if (patch < v.patch)
            return true;
        if (patch > v.patch)
            return false;
        return false;
    }
    bool operator>(const FeatureVersion& v) const { return !(*this < v) && !(*this == v); }
    bool operator>=(const FeatureVersion& v) const { return !(*this < v); }
    bool operator<=(const FeatureVersion& v) const { return !(*this > v); }
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