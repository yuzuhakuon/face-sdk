#include "gawrs_face/services/detector/scrfd.h"
#include "gawrs_face/services/calculator/nms.h"
#include "gawrs_face/types/anchor.h"

#ifdef NCNN_INMEMORY
#include "scrfd.id.h"
#include "scrfd.mem.h"
#endif

namespace gawrs_face
{
namespace internal
{
std::vector<Anchor> generateAnchors(int baseSize, std::vector<float> aspectRatios, std::vector<float> scales)
{
    int numAspectRatios = aspectRatios.size();
    int numScales = scales.size();
    std::vector<Anchor> anchors;

    const float cx = 0;
    const float cy = 0;
    for (int i = 0; i < numAspectRatios; i++)
    {
        float ar = aspectRatios[i];
        float w = baseSize * std::sqrt(ar);
        float h = baseSize / std::sqrt(ar);

        for (int j = 0; j < numScales; j++)
        {
            float scale = scales[j];
            float sW = w * scale;
            float sH = h * scale;

            Anchor anchor{
                .xCenter = cx,
                .yCenter = cy,
                .width = sW,
                .height = sH,
            };
            anchors.push_back(anchor);
        }
    }

    return anchors;
}

void generateProposals(const std::vector<Anchor>& anchors, int featStride, float probThreshold,
                       const ncnn::Mat& scoreBlob, const ncnn::Mat& bboxBlob, const ncnn::Mat& kpsBlob,
                       DetectionResult& proposals, SCRFDScaleParams scaleParams)
{
    int numAnchors = anchors.size();
    int width = scoreBlob.w;
    int height = scoreBlob.h;

    for (int q = 0; q < numAnchors; ++q)
    {
        const auto anchor = anchors[q];
        const ncnn::Mat score = scoreBlob.channel(q);
        const ncnn::Mat bbox = bboxBlob.channel_range(q * 4, 4);
        const ncnn::Mat kps = kpsBlob.channel_range(q * 10, 10);

        float cy = anchor.yCenter;
        for (int i = 0; i < height; ++i)
        {
            float cx = anchor.xCenter;
            for (int j = 0; j < width; ++j)
            {
                int index = i * width + j;
                float scoreValue = score[index];
                if (scoreValue >= probThreshold)
                {
                    float dx = bbox.channel(0)[index] * featStride;
                    float dy = bbox.channel(1)[index] * featStride;
                    float dw = bbox.channel(2)[index] * featStride;
                    float dh = bbox.channel(3)[index] * featStride;

                    float x1 = (cx - dx) / scaleParams.ratio;
                    float y1 = (cy - dy) / scaleParams.ratio;
                    float x2 = (cx + dw) / scaleParams.ratio;
                    float y2 = (cy + dh) / scaleParams.ratio;

                    RelativeBoundingBox relativeBoundingBox{
                        .xmin = x1, .ymin = y1, .width = x2 - x1 + 1, .height = y2 - y1 + 1};

                    RelativeKeypoints relativeKeypoints;
                    for (int k = 0; k < 5; ++k)
                    {
                        float kpX = cx + kps.channel(2 * k)[index] * featStride;
                        float kpY = cy + kps.channel(2 * k + 1)[index] * featStride;
                        relativeKeypoints.push_back({kpX / scaleParams.ratio, kpY / scaleParams.ratio});
                    }

                    Detection detection{.label = 0,
                                        .score = scoreValue,
                                        .boundingBox = relativeBoundingBox,
                                        .keypoints = relativeKeypoints};

                    proposals.push_back(detection);
                }
                cx += featStride;
            }
            cy += featStride;
        }
    }
}
} // namespace internal

bool SCRFD::loadModelInMemory()
{
#ifdef NCNN_INMEMORY
    return INCNNInference::loadModelInMemory(scrfd_param_bin, scrfd_bin);
#endif
    return false;
}

ncnn::Mat SCRFD::preprocess(const ncnn::Mat& inImage, SCRFDScaleParams& scaleParams)
{
    int width = inImage.w;
    int height = inImage.h;

    const int targetSize = 256;

    // pad to multiple of 32
    int w = width;
    int h = height;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)targetSize / w;
        w = targetSize;
        h = h * scale;
    }
    else
    {
        scale = (float)targetSize / h;
        h = targetSize;
        w = w * scale;
    }

    ncnn::Mat resizedImage;
    ncnn::resize_bicubic(inImage, resizedImage, w, h);

    // pad to target_size rectangle
    int wpad = (w + 31) / 32 * 32 - w;
    int hpad = (h + 31) / 32 * 32 - h;
    ncnn::Mat padImage;
    // TODO: maybe something wrong
    ncnn::copy_make_border(resizedImage, padImage, 0, hpad, 0, wpad, ncnn::BORDER_CONSTANT, 0.f);

    const float meanVals[3] = {127.5f, 127.5f, 127.5f};
    const float normVals[3] = {1 / 128.f, 1 / 128.f, 1 / 128.f};
    padImage.substract_mean_normalize(meanVals, normVals);

    scaleParams.ratio = scale;
    scaleParams.dstW = padImage.w;
    scaleParams.dstH = padImage.h;

    return padImage;
}

DetectionResult SCRFD::doInference(const ncnn::Mat& inImage, float probThreshold, float nmsThreshold)
{
    SCRFDScaleParams scaleParams;
    auto blob = preprocess(inImage, scaleParams);
    ncnn::Extractor ex = net_.create_extractor();
    if (inMemory_)
    {
#ifdef NCNN_INMEMORY
        ex.input(scrfd_param_id::BLOB_input_1, blob);
#endif
    }
    else
    {
        ex.input("input.1", blob);
    }

    DetectionResult proposals;
    {
        ncnn::Mat scoreBlob, bboxBlob, kpsBlob;
        if (inMemory_)
        {
#ifdef NCNN_INMEMORY
            ex.extract(scrfd_param_id::BLOB_score_8, scoreBlob);
            ex.extract(scrfd_param_id::BLOB_bbox_8, bboxBlob);
            ex.extract(scrfd_param_id::BLOB_kps_8, kpsBlob);
#endif
        }
        else
        {
            ex.extract("score_8", scoreBlob);
            ex.extract("bbox_8", bboxBlob);
            ex.extract("kps_8", kpsBlob);
        }

        const int baseSize = 16;
        const int featStride = 8;
        const std::vector<float> ratios = {1.0f};
        const std::vector<float> scales = {1.0f, 2.0f};

        auto anchors = internal::generateAnchors(baseSize, ratios, scales);
        internal::generateProposals(anchors, featStride, probThreshold, scoreBlob, bboxBlob, kpsBlob, proposals,
                                    scaleParams);
    }
    {
        ncnn::Mat scoreBlob, bboxBlob, kpsBlob;
        if (inMemory_)
        {
#ifdef NCNN_INMEMORY
            ex.extract(scrfd_param_id::BLOB_score_16, scoreBlob);
            ex.extract(scrfd_param_id::BLOB_bbox_16, bboxBlob);
            ex.extract(scrfd_param_id::BLOB_kps_16, kpsBlob);
#endif
        }
        else
        {
            ex.extract("score_16", scoreBlob);
            ex.extract("bbox_16", bboxBlob);
            ex.extract("kps_16", kpsBlob);
        }

        const int baseSize = 64;
        const int featStride = 16;
        const std::vector<float> ratios = {1.0f};
        const std::vector<float> scales = {1.0f, 2.0f};

        auto anchors = internal::generateAnchors(baseSize, ratios, scales);
        internal::generateProposals(anchors, featStride, probThreshold, scoreBlob, bboxBlob, kpsBlob, proposals,
                                    scaleParams);
    }
    {
        ncnn::Mat scoreBlob, bboxBlob, kpsBlob;
        if (inMemory_)
        {
#ifdef NCNN_INMEMORY

            ex.extract(scrfd_param_id::BLOB_score_32, scoreBlob);
            ex.extract(scrfd_param_id::BLOB_bbox_32, bboxBlob);
            ex.extract(scrfd_param_id::BLOB_kps_32, kpsBlob);
#endif
        }
        else
        {
            ex.extract("score_32", scoreBlob);
            ex.extract("bbox_32", bboxBlob);
            ex.extract("kps_32", kpsBlob);
        }

        const int baseSize = 256;
        const int featStride = 32;
        const std::vector<float> ratios = {1.0f};
        const std::vector<float> scales = {1.0f, 2.0f};

        auto anchors = internal::generateAnchors(baseSize, ratios, scales);
        internal::generateProposals(anchors, featStride, probThreshold, scoreBlob, bboxBlob, kpsBlob, proposals,
                                    scaleParams);
    }

    auto detCmpByScore = [](const Detection& a, const Detection& b) { return a.score < b.score; };
    std::sort(proposals.begin(), proposals.end(), detCmpByScore);

    DetectionResult outDetections;
    const NonMaxSuppressionOptions nmsOptions{
        .minSuppressionThreshold = nmsThreshold,
        .overlapType = OverlapType::INTERSECTION_OVER_UNION,
        .returnEmptyDetections = true,
        .algorithm = NmsAlgorithm::DEFAULT,
    };
    NonMaxSuppressionCalculator nms(nmsOptions);
    nms(proposals, outDetections);

    return outDetections;
}
} // namespace gawrs_face