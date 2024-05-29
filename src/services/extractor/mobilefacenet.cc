#include "gawrs_face/services/extractor/mobilefacenet.h"

#ifdef NCNN_INMEMORY
#include "mbf.id.h"
#include "mbf.mem.h"
#endif

namespace gawrs_face
{
bool MobileFaceNet::loadModelInMemory()
{
#ifdef NCNN_INMEMORY
    return INCNNInference::loadModelInMemory(mbf_param_bin, mbf_bin);
#endif
    return false;
}

std::vector<float> MobileFaceNet::doInference(ncnn::Mat inImage)
{
    const float meanVals[3] = {127.5f, 127.5f, 127.5f};
    const float normVals[3] = {1 / 128.f, 1 / 128.f, 1 / 128.f};
    inImage.substract_mean_normalize(meanVals, normVals);

    ncnn::Mat embedding;
    ncnn::Extractor ex = net_.create_extractor();
    if (inMemory_)
    {
#ifdef NCNN_INMEMORY
        ex.input(mbf_param_id::BLOB_data, inImage);
        ex.extract(mbf_param_id::BLOB_feature, embedding);
#endif
    }
    else
    {
        ex.input(inputName_, inImage);
        ex.extract(embName_, embedding);
    }

    std::vector<float> feature(kEmbeddingDims[1]);
    std::memcpy(feature.data(), embedding.data, kEmbeddingDims[1] * sizeof(float));

    return feature;
}
} // namespace gawrs_face