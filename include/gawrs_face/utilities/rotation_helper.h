#pragma once
#include <tuple>

#include "gawrs_face/sdk_interface/sdk_interface.h"
#include "gawrs_face/types/detection.h"

namespace gawrs_face
{
/**
 * @brief Rotate a point (px, py) around a center (cx, cy) by a given angle in radians.
 *
 * @param cx Coordinates of the center x of rotation
 * @param cy  Coordinates of the center y of rotation
 * @param angle Rotation angle in radians
 * @param px Coordinates of the point to rotate
 * @param py Coordinates of the point to rotate
 * @return std::tuple<float, float> Coordinates of the point after rotation
 */
std::tuple<float, float> rotatePoint(float cx, float cy, float angle, float px, float py);

/**
 * @brief Rotate a point (px, py) around a center (cx, cy) by 90 degrees clockwise.
 *
 * @param cx Coordinates of the center of rotation
 * @param cy Coordinates of the center of rotation
 * @param px Coordinates of the point to rotate
 * @param py Coordinates of the point to rotate
 * @return std::tuple<float, float> Coordinates of the point after 90 degrees clockwise rotation
 */
std::tuple<float, float> rotatePoint90Cw(float cx, float cy, float px, float py);

/**
 * @brief Rotate a point (px, py) around a center (cx, cy) by 90 degrees counterclockwise.
 *
 * @param cx Coordinates of the center of rotation
 * @param cy Coordinates of the center of rotation
 * @param px Coordinates of the point to rotate
 * @param py Coordinates of the point to rotate
 * @return std::tuple<float, float> Coordinates of the point after 90 degrees counterclockwise rotation
 */
std::tuple<float, float> rotatePoint90Ccw(float cx, float cy, float px, float py);

/**
 * @brief Horizontally flip a point (px, py) around a vertical axis at cx.
 *
 * @param cx X-coordinate of the vertical axis (center of flip)
 * @param px Coordinates of the point to flip
 * @param py Coordinates of the point to flip
 * @return std::tuple<float, float> Coordinates of the point after horizontal flip
 */
std::tuple<float, float> horizontalFlip(float cx, float px, float py);

/**
 * @brief Vertically flip a point (px, py) around a horizontal axis at cy.
 *
 * @param cy Y-coordinate of the horizontal axis (center of flip)
 * @param px Coordinates of the point to flip
 * @param py Coordinates of the point to flip
 * @return std::tuple<float, float> Coordinates of the point after vertical flip
 */
std::tuple<float, float> verticalFlip(float cy, float px, float py);

RelativeBoundingBox rotateBoundingBox(const RelativeBoundingBox& box, RotationModel rotation);
RelativeKeypoint rotateKeypoint(const RelativeKeypoint& keypoint, RotationModel rotation);
RelativeKeypoints rotateKeypoints(const RelativeKeypoints& keypoints, RotationModel rotation);

Detection rotateDetectionWithRelative(const Detection& detection, RotationModel rotation);
Detection undoRotateDetectionWithRelative(const Detection& detection, RotationModel rotation);
RotationModel undoRotation(RotationModel model);
} // namespace gawrs_face