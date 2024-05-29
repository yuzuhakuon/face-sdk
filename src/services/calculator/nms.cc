#include "gawrs_face/services/calculator/nms.h"
#include "gawrs_face/types/rectangle.h"

namespace gawrs_face
{
namespace internal
{
using IndexedScores = std::vector<std::pair<int, float>>;

inline bool sortBySecond(const std::pair<int, float>& indexedScore0, const std::pair<int, float>& indexedScore1)
{
    return (indexedScore0.second > indexedScore1.second);
}

// Computes an overlap similarity between two rectangles. Similarity measure is
// defined by overlap_type parameter.
float overlapSimilarity(const OverlapType overlapType, const RectangleF& rect1, const RectangleF& rect2)
{
    if (!rect1.intersects(rect2))
        return 0.0f;

    const float intersectionArea = RectangleF(rect1).intersect(rect2).area();
    float normalization;

    switch (overlapType)
    {
        case OverlapType::JACCARD:
            normalization = RectangleF(rect1).unionArea(rect2).area();
            break;
        case OverlapType::MODIFIED_JACCARD:
            normalization = rect2.area();
            break;
        case OverlapType::INTERSECTION_OVER_UNION:
            normalization = rect1.area() + rect2.area() - intersectionArea;
            break;
        default:
            normalization = RectangleF(rect1).unionArea(rect2).area();
            break;
    }

    return normalization > 0.0f ? intersectionArea / normalization : 0.0f;
}

// Computes an overlap similarity between two locations by first extracting the
// relative box from the location. It assumes that a relative-box representation
// is already available in the location, and therefore frame width and height
// are not needed for further normalization.
float overlapSimilarity(const OverlapType overlapType, const RelativeBoundingBox& box1, const RelativeBoundingBox& box2)
{
    const auto rect1 = RectangleF(box1.xmin, box1.ymin, box1.width, box1.height);
    const auto rect2 = RectangleF(box2.xmin, box2.ymin, box2.width, box2.height);
    return overlapSimilarity(overlapType, rect1, rect2);
}

void nonMaxSuppression(const IndexedScores& indexedScores, const DetectionResult& detections, int maxNumDetections,
                       const NonMaxSuppressionOptions& options, DetectionResult& outputDetections)
{
    std::vector<Detection> retainedLocations;
    retainedLocations.reserve(maxNumDetections);
    for (const auto& indexedScore : indexedScores)
    {
        const auto& detection = detections[indexedScore.first];
        if (options.minScoreThreshold > 0 && detection.score < options.minScoreThreshold)
        {
            break;
        }

        const auto& box1 = std::get<RelativeBoundingBox>(detection.boundingBox);
        bool suppressed = false;
        for (const auto& retainedLocation : retainedLocations)
        {
            const auto& box2 = std::get<RelativeBoundingBox>(retainedLocation.boundingBox);
            float similarity = overlapSimilarity(options.overlapType, box2, box1);

            if (similarity > options.minSuppressionThreshold)
            {
                suppressed = true;
                break;
            }
        }

        if (!suppressed)
        {
            outputDetections.push_back(detection);
            retainedLocations.push_back(detection);
        }
        if (outputDetections.size() >= maxNumDetections)
        {
            break;
        }
    }
}

void weightedNonMaxSuppression(const IndexedScores& indexedScores, const DetectionResult& detections,
                               int maxNumDetections, const NonMaxSuppressionOptions& options,
                               DetectionResult& outputDetections)
{
    IndexedScores remainedIndexedScores;
    remainedIndexedScores.assign(indexedScores.begin(), indexedScores.end());

    IndexedScores remained;
    IndexedScores candidates;
    outputDetections.clear();

    while (!remainedIndexedScores.empty())
    {
        const int originalIndexedScoresSize = remainedIndexedScores.size();
        const auto& detection = detections[remainedIndexedScores[0].first];
        if (options.minScoreThreshold > 0 && detection.score < options.minScoreThreshold)
        {
            break;
        }
        remained.clear();
        candidates.clear();

        const auto& box1 = std::get<RelativeBoundingBox>(detection.boundingBox);
        for (const auto& indexedScore : remainedIndexedScores)
        {
            const auto& box2 = std::get<RelativeBoundingBox>(detections[indexedScore.first].boundingBox);
            float similarity = overlapSimilarity(options.overlapType, box2, box1);
            if (similarity > options.minScoreThreshold)
            {
                candidates.push_back(indexedScore);
            }
            else
            {
                remained.push_back(indexedScore);
            }
        }

        auto weightedDetection = detection;
        if (!candidates.empty())
        {
            const int numKeypoints = std::get<RelativeKeypoints>(detection.keypoints).size();
            std::vector<float> keypoints(numKeypoints * 2);
            float wXMin = 0.0f;
            float wYMin = 0.0f;
            float wXMax = 0.0f;
            float wYMax = 0.0f;
            float totalScore = 0.0f;
            for (const auto& candidate : candidates)
            {
                totalScore += candidate.second;
                const auto& bbox = std::get<RelativeBoundingBox>(detections[candidate.first].boundingBox);
                wXMin += bbox.xmin * candidate.second;
                wYMin += bbox.ymin * candidate.second;
                wXMax += (bbox.xmin + bbox.width) * candidate.second;
                wYMax += (bbox.ymin + bbox.height) * candidate.second;

                const auto& relativeKeypoints = std::get<RelativeKeypoints>(detections[candidate.first].keypoints);
                for (int i = 0; i < numKeypoints; i++)
                {
                    keypoints[i * 2] = relativeKeypoints[i].x() * candidate.second;
                    keypoints[i * 2 + 1] = relativeKeypoints[i].y() * candidate.second;
                }
            }
            auto weightedLocation = std::get<RelativeBoundingBox>(weightedDetection.boundingBox);
            weightedLocation.xmin = wXMin / totalScore;
            weightedLocation.ymin = wYMin / totalScore;
            weightedLocation.width = (wXMax / totalScore) - weightedLocation.xmin;
            weightedLocation.height = (wYMax / totalScore) - weightedLocation.ymin;

            auto& relativeKeypoints = std::get<RelativeKeypoints>(weightedDetection.keypoints);
            for (int i = 0; i > numKeypoints; i++)
            {
                relativeKeypoints[i].set(keypoints[i * 2] / totalScore, keypoints[i * 2 + 1] / totalScore);
            }
        }

        outputDetections.push_back(weightedDetection);
        // Breaks the loop if the size of indexed scores doesn't change after an
        // iteration.
        if (originalIndexedScoresSize == remained.size())
        {
            break;
        }
        else
        {
            remainedIndexedScores = std::move(remained);
        }
    }
}
} // namespace internal

NonMaxSuppressionCalculator::NonMaxSuppressionCalculator(const NonMaxSuppressionOptions& options) : options_(options) {}

void NonMaxSuppressionCalculator::operator()(const std::vector<Detection>& inputDetections,
                                             std::vector<Detection>& retainedDetections)
{
    internal::IndexedScores indexedScores;
    indexedScores.reserve(inputDetections.size());
    for (int i = 0; i < inputDetections.size(); i++)
    {
        indexedScores.push_back(std::make_pair(i, inputDetections[i].score));
    }

    std::sort(indexedScores.begin(), indexedScores.end(), internal::sortBySecond);

    const int maxNumDetections =
        (options_.maxNumDetections > -1) ? options_.maxNumDetections : static_cast<int>(indexedScores.size());

    if (options_.algorithm == NmsAlgorithm::DEFAULT)
    {
        internal::nonMaxSuppression(indexedScores, inputDetections, //
                                    maxNumDetections, options_, retainedDetections);
    }
    else
    {
        internal::weightedNonMaxSuppression(indexedScores, inputDetections, //
                                            maxNumDetections, options_, retainedDetections);
    }
}

} // namespace gawrs_face