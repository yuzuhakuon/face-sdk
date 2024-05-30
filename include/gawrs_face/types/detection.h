#pragma once
#include <cstdint>
#include <vector>

#include "point2.h"

namespace gawrs_face
{
template <class T = int32_t>
struct BoundingBoxT
{
    T xmin;
    T ymin;
    T width;
    T height;
};

using RelativeKeypoint = Point2<float>;
using AbsoluteKeypoint = Point2<int>;
using RelativeBoundingBox = BoundingBoxT<float>;
using AbsoluteBoundingBox = BoundingBoxT<int>;
using RelativeKeypoints = std::vector<RelativeKeypoint>;
using AbsoluteKeypoints = std::vector<AbsoluteKeypoint>;

using BoundingBox = std::variant<RelativeBoundingBox, AbsoluteBoundingBox>;
using Keypoints = std::variant<RelativeKeypoints, AbsoluteKeypoints>;


struct Detection
{
    // The label id in the classification model output.
    int32_t label;
    // The score for this category, e.g. (but not necessarily) a probability in
    // [0,1].
    float score;
    // The bounding box location.
    BoundingBox boundingBox;
    // A vector of keypoint.
    Keypoints keypoints;
};

using DetectionResult = std::vector<Detection>;

} // namespace gawrs_face