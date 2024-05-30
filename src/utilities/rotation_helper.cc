#include <cmath>

#include "gawrs_face/types/detection.h"
#include "gawrs_face/utilities/rotation_helper.h"

namespace gawrs_face
{
std::tuple<float, float> rotatePoint(float cx, float cy, float angle, float px, float py)
{
    // Translate point to origin
    float translatedX = px - cx;
    float translatedY = py - cy;

    // Apply rotation
    float newX = translatedX * std::cos(angle) - translatedY * std::sin(angle);
    float newY = translatedX * std::sin(angle) + translatedY * std::cos(angle);

    //  Translate point back
    newX += cx;
    newY += cy;

    return std::make_tuple(newX, newY);
}

std::tuple<float, float> rotatePoint90Cw(float cx, float cy, float px, float py)
{
    float newX = -(py - cy) + cx;
    float newY = (px - cx) + cy;

    return std::make_tuple(newX, newY);
}

std::tuple<float, float> rotatePoint90Ccw(float cx, float cy, float px, float py)
{
    float newX = (py - cy) + cx;
    float newY = -(px - cx) + cy;

    return std::make_tuple(newX, newY);
}

std::tuple<float, float> horizontalFlip(float cx, float px, float py)
{
    float newX = cx - (px - cx);
    float newY = py;

    return std::make_tuple(newX, newY);
}

std::tuple<float, float> verticalFlip(float cy, float px, float py)
{
    float newX = px;
    float newY = cy - (py - cy);

    return std::make_tuple(newX, newY);
}

RelativeBoundingBox rotateBoundingBox(const RelativeBoundingBox& box, RotationModel rotation)
{
    float left = box.xmin;
    float top = box.ymin;
    float right = box.xmin + box.width;
    float bottom = box.ymin + box.height;
    switch (rotation)
    {
        case RotationModel::GF_ROTATE_0:
            return box;
        case RotationModel::GF_ROTATE_90:
        {
            auto [xmin, ymin] = rotatePoint90Cw(0.5, 0.5, left, bottom);
            return {xmin, ymin, box.height, box.width};
        }
        case RotationModel::GF_ROTATE_180:
        {
            auto [x, y] = rotatePoint90Cw(0.5, 0.5, right, bottom);
            auto [xmin, ymin] = rotatePoint90Cw(0.5, 0.5, x, y);
            return {xmin, ymin, box.width, box.height};
        }
        case RotationModel::GF_ROTATE_270:
        {
            auto [xmin, ymin] = rotatePoint90Ccw(0.5, 0.5, right, top);
            return {xmin, ymin, box.height, box.width};
        }
        case RotationModel::GF_HorizontalFlip:
        {
            auto [xmin, ymin] = horizontalFlip(0.5, right, top);
            return {xmin, ymin, box.width, box.height};
        }
        case RotationModel::GF_VerticalFlip:
        {
            auto [xmin, ymin] = verticalFlip(0.5, left, bottom);
            return {xmin, ymin, box.width, box.height};
        }
        default:
            return box;
    }
}

RelativeKeypoint rotateKeypoint(const RelativeKeypoint& keypoint, RotationModel rotation)
{
    switch (rotation)
    {
        case RotationModel::GF_ROTATE_0:
            return keypoint;
        case RotationModel::GF_ROTATE_90:
        {
            auto [x, y] = rotatePoint90Cw(0.5, 0.5, keypoint.x(), keypoint.y());
            return {x, y};
        }
        case RotationModel::GF_ROTATE_180:
        {
            auto [x_, y_] = rotatePoint90Cw(0.5, 0.5, keypoint.x(), keypoint.y());
            auto [x, y] = rotatePoint90Cw(0.5, 0.5, x_, y_);
            return {x, y};
        }
        case RotationModel::GF_ROTATE_270:
        {
            auto [x, y] = rotatePoint90Ccw(0.5, 0.5, keypoint.x(), keypoint.y());
            return {x, y};
        }
        case RotationModel::GF_HorizontalFlip:
        {
            auto [x, y] = horizontalFlip(0.5, keypoint.x(), keypoint.y());
            return {x, y};
        }
        case RotationModel::GF_VerticalFlip:
        {
            auto [x, y] = verticalFlip(0.5, keypoint.x(), keypoint.y());
            return {x, y};
        }
        default:
            return keypoint;
    }
}

RelativeKeypoints rotateKeypoints(const RelativeKeypoints& keypoints, RotationModel rotation)
{
    RelativeKeypoints rotatedKeypoints;
    for (auto& keypoint : keypoints)
    {
        auto p = rotateKeypoint(keypoint, rotation);
        rotatedKeypoints.push_back(p);
    }
    return rotatedKeypoints;
}

Detection rotateDetectionWithRelative(const Detection& detection, RotationModel rotation)
{
    auto& keypoints = std::get<RelativeKeypoints>(detection.keypoints);
    auto rotatedKeypoints = rotateKeypoints(keypoints, rotation);
    auto& bbox = std::get<RelativeBoundingBox>(detection.boundingBox);
    auto rotatedBoundingBox = rotateBoundingBox(bbox, rotation);
    return Detection{
        .label = detection.label,
        .score = detection.score,
        .boundingBox = std::move(rotatedBoundingBox),
        .keypoints = std::move(rotatedKeypoints),
    };
}

Detection undoRotateDetectionWithRelative(const Detection& detection, RotationModel rotation)
{
    if (rotation == RotationModel::GF_ROTATE_0)
    {
        return detection;
    }

    rotation = undoRotation(rotation);
    auto newDetection = rotateDetectionWithRelative(detection, rotation);

    return newDetection;
}

RotationModel undoRotation(RotationModel model)
{
    switch (model)
    {
        case RotationModel::GF_ROTATE_0:
            return RotationModel::GF_ROTATE_0;
        case RotationModel::GF_ROTATE_90:
            return RotationModel::GF_ROTATE_270;
        case RotationModel::GF_ROTATE_180:
            return RotationModel::GF_ROTATE_180;
        case RotationModel::GF_ROTATE_270:
            return RotationModel::GF_ROTATE_90;
        default:
            return RotationModel::GF_ROTATE_0;
    }
}
} // namespace gawrs_face