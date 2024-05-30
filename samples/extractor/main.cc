#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "gawrs_face/services/extractor/mobilefacenet.h"
#include "gawrs_face/utilities/similarity.h"

using namespace gawrs_face;

ncnn::Mat readImage(std::string imgPath)
{
    cv::Mat cvImage = cv::imread(imgPath);
    int width = cvImage.cols;
    int height = cvImage.rows;
    int channels = cvImage.channels();
    ncnn::Mat image = ncnn::Mat::from_pixels(cvImage.data, ncnn::Mat::PIXEL_BGR2RGB, width, height);
    return image;
}

int main()
{
    auto imageA = readImage("./samples/data/images/a1_crop.jpg");
    auto imageB = readImage("./samples/data/images/a2_crop.jpg");
    auto imageC = readImage("./samples/data/images/b2_crop.jpg");

    MobileFaceNet model;
    if (!model.loadModelInMemory())
    {
        std::cout << "load model failed." << std::endl;
        return -1;
    }
    auto featureA = model.doInference(imageA.clone());
    auto featureB = model.doInference(imageB.clone());
    auto featureC = model.doInference(imageC.clone());

    float cosineAB = cosineSimilarity(featureA.data(), featureB.data(), featureA.size());
    float cosineAC = cosineSimilarity(featureA.data(), featureC.data(), featureA.size());
    float cosineBC = cosineSimilarity(featureB.data(), featureC.data(), featureB.size());

    std::cout << "cosineAB(same person): " << cosineAB << std::endl;
    std::cout << "cosineAC(different person): " << cosineAC << std::endl;
    std::cout << "cosineBC(different person): " << cosineBC << std::endl;

    return 0;
}