#pragma once
#include <datareader.h>
#include <net.h>
#include <string>

namespace gawrs_face
{
class DataReaderFromEmpty : public ncnn::DataReader
{
public:
    virtual int scan(const char* format, void* p) const { return 0; }
    virtual size_t read(void* buf, size_t size) const
    {
        memset(buf, 0, size);
        return size;
    }
};

class INCNNInference
{
public:
    std::string summary();

    bool loadModel(const char* paramPath);
    bool loadModel(const char* paramPath, const char* modelPath);
    bool loadModelInMemory(const unsigned char* paramMem, const unsigned char* binMem);

    ncnn::Net& net() { return net_; }
    const ncnn::Net& net() const { return net_; }

protected:
    ncnn::Net net_;
    bool inMemory_{false};
};
} // namespace gawrs_face