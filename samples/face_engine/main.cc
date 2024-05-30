#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "gawrs_face/common/config.h"
#include "gawrs_face/sdk_interface/sdk_types.h"
#include "gawrs_face/services/face_engine.h"
#include "gawrs_face/types/face_feature.h"
#include "gawrs_face/utilities/similarity.h"

using namespace gawrs_face;

int main()
{
    auto imageA = cv::imread("./samples/data/images/a1.png");
    auto imageB = cv::imread("./samples/data/images/a2.png");
    auto imageC = cv::imread("./samples/data/images/b2.png");

    if (imageA.empty() || imageB.empty() || imageC.empty())
    {
        std::cout << "Failed to read image" << std::endl;
        return -1;
    }

    FaceEngineConfig config{
        .combinedMask = RuntimeModel::GF_FACE_RECOGNITION | RuntimeModel::GF_FACE_DETECTION,
    };
    auto engine = FaceEngine();
    engine.initialize(config);

    std::vector<Detection> detsA, detsB, detsC;
    engine.detectFace(imageA.data, imageA.cols, imageA.rows, ImageFormat::GF_BGR24, detsA);
    engine.detectFace(imageB.data, imageB.cols, imageB.rows, ImageFormat::GF_BGR24, detsB);
    engine.detectFace(imageC.data, imageC.cols, imageC.rows, ImageFormat::GF_BGR24, detsC);

    if (detsA.size() > 0 && detsB.size() > 0 && detsC.size() > 0)
    {
        FaceFeaturePacked packedA, packedB, packedC;
        engine.extractFaceFeature(imageA.data, imageA.cols, imageA.rows, ImageFormat::GF_BGR24, detsA[0], packedA);
        engine.extractFaceFeature(imageB.data, imageB.cols, imageB.rows, ImageFormat::GF_BGR24, detsB[0], packedB);
        engine.extractFaceFeature(imageC.data, imageC.cols, imageC.rows, ImageFormat::GF_BGR24, detsC[0], packedC);
        float cosineAB = cosineSimilarity(packedA.feature, packedB.feature, kFeatureSize);
        float cosineAC = cosineSimilarity(packedA.feature, packedC.feature, kFeatureSize);
        float cosineBC = cosineSimilarity(packedB.feature, packedC.feature, kFeatureSize);

        std::cout << "cosineAB(same person): " << cosineAB << std::endl;
        std::cout << "cosineAC(different person): " << cosineAC << std::endl;
        std::cout << "cosineBC(different person): " << cosineBC << std::endl;
    }

    return 0;
}