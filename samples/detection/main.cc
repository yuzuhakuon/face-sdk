#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "gawrs_face/services/detector/scrfd.h"
#include "gawrs_face/types/detection.h"
#include "gawrs_face/utilities/face_align.h"

using namespace gawrs_face;

void drawImage(cv::Mat& image, const Detection& det)
{
    auto bbox = std::get<RelativeBoundingBox>(det.boundingBox);
    auto kps = std::get<RelativeKeypoints>(det.keypoints);
    cv::rectangle(image, cv::Rect(bbox.xmin, bbox.ymin, bbox.width, bbox.height), cv::Scalar(0, 255, 0), 2);
    for (auto p : kps)
    {
        cv::circle(image, cv::Point(p.x(), p.y()), 2, cv::Scalar(0, 0, 255), 2);
    }
}

int main()
{
    std::string imagePath = "./samples/data/images/a1.png";
    cv::Mat image = cv::imread(imagePath);
    if (image.empty())
    {
        std::cout << "read image failed." << std::endl;
        return -1;
    }
    SCRFD detector;
    if (!detector.loadModelInMemory())
    {
        std::cout << "load model failed." << std::endl;
        return -1;
    }

    ncnn::Mat inImage = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_BGR2RGB, image.cols, image.rows);
    auto imageCpy = inImage.clone();
    auto result = detector.doInference(inImage.clone());
    std::cout << "detected " << result.size() << " faces." << std::endl;
    for (auto& det : result)
    {
        drawImage(image, det);

        auto crop_ = normCrop(imageCpy, det, 112, 112);
        cv::Mat crop(crop_.h, crop_.w, CV_8UC3);
        crop_.to_pixels(crop.data, ncnn::Mat::PIXEL_RGB2BGR);
        cv::imwrite("tmp/result_crop.jpg", crop);
    }
    cv::imwrite("tmp/detection.jpg", image);
    return 0;
}