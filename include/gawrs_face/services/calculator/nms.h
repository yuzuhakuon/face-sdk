#pragma once
#include <cstdint>

#include "gawrs_face/types/detection.h"

namespace gawrs_face
{
enum class OverlapType : int32_t
{
    UNSPECIFIED_OVERLAP_TYPE = 0,
    JACCARD = 1,
    MODIFIED_JACCARD = 2,
    INTERSECTION_OVER_UNION = 3,
};
// Algorithms that can be used to apply non-maximum suppression.
enum class NmsAlgorithm : int32_t
{
    DEFAULT = 0,
    // Only supports relative bounding box for weighted NMS.
    WEIGHTED = 1,
};

struct NonMaxSuppressionOptions
{
    int32_t numDetections{1};
    int32_t maxNumDetections{-1};

    float minScoreThreshold{-1.0};
    float minSuppressionThreshold{1.0};

    OverlapType overlapType{OverlapType::JACCARD};

    // Whether to put empty detection vector in output stream.
    bool returnEmptyDetections{false};

    NmsAlgorithm algorithm{NmsAlgorithm::DEFAULT};
};
class NonMaxSuppressionCalculator final
{
public:
    NonMaxSuppressionCalculator(const NonMaxSuppressionOptions& options);
    void operator()(const std::vector<Detection>& inputDetections, std::vector<Detection>& retainedDetections);

public:
    const NonMaxSuppressionOptions& options() const { return options_; }
    NonMaxSuppressionOptions& options() { return options_; }

private:
    NonMaxSuppressionOptions options_;
};

} // namespace gawrs_face