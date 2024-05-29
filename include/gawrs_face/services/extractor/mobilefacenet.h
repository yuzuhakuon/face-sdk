#pragma once

#include "gawrs_face/common/config.h"
#include "gawrs_face/services/ncnn_inference.h"

namespace gawrs_face
{

class MobileFaceNet : public INCNNInference
{
public:
    bool loadModelInMemory();
    std::vector<float> doInference(ncnn::Mat inImage);

public:
    static constexpr std::array<int, 4> kInputDims{1, kAlignedFaceSize, kAlignedFaceSize, 3};
    static constexpr std::array<int, 2> kEmbeddingDims{1, kFeatureSize};
    static constexpr std::array<int, 2> kInputShape{kAlignedFaceSize, kAlignedFaceSize};

private:
    const char* inputName_ = "data";
    const char* embName_ = "feature";
};
} // namespace gawrs_face