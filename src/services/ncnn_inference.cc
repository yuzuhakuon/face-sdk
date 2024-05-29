#include <sstream>

#include "gawrs_face/services/ncnn_inference.h"

namespace gawrs_face
{
std::string INCNNInference::summary()
{
    std::stringstream ss;
    ss << "Summary: \n";
    auto& inputNames = net_.input_names();
    auto& outputNames = net_.output_names();
    for (auto name : inputNames)
    {
        ss << "  input name: " << name << "\n";
    }

    for (auto name : outputNames)
    {
        ss << "  output name: " << name << "\n";
    }

    return ss.str();
}

bool INCNNInference::loadModel(const char* paramPath)
{
    int ret1 = net_.load_param(paramPath);
    DataReaderFromEmpty dr;
    net_.load_model(dr);
    inMemory_ = false;

    return true;
}

bool INCNNInference::loadModel(const char* paramPath, const char* modelPath)
{
    int ret1 = net_.load_param(paramPath);
    int ret2 = net_.load_model(modelPath);
    inMemory_ = false;

    return true;
}

bool INCNNInference::loadModelInMemory(const unsigned char* paramMem, const unsigned char* binMem)
{
    net_.load_param(paramMem);
    net_.load_model(binMem);
    inMemory_ = true;

    return true;
}
} // namespace gawrs_face