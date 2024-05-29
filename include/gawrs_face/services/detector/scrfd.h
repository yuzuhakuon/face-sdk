#pragma once
#include <vector>

#include "gawrs_face/services/ncnn_inference.h"
#include "gawrs_face/types/detection.h"

namespace gawrs_face
{
struct SCRFDScaleParams
{
    float ratio;
    int dstW;
    int dstH;
};

class SCRFD : public INCNNInference
{
public:
    bool loadModelInMemory();
    std::vector<Detection> doInference(const ncnn::Mat& inImage, float probThreshold, float nmsThreshold);

    static constexpr int numKeypoints() { return 5; }
    static constexpr int numValuesPerKeypoint() { return 2; }

private:
    ncnn::Mat preprocess(const ncnn::Mat& inImage, SCRFDScaleParams& scaleParams);
};

} // namespace gawrs_face