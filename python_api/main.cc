#include <cstdint>
#include <fmt/format.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "gawrs_face/common/config.h"
#include "gawrs_face/sdk_interface/sdk_types.h"
#include "gawrs_face/services/face_engine.h"
#include "gawrs_face/types/detection.h"
#include "gawrs_face/types/face_feature.h"

namespace py = pybind11;
using namespace gawrs_face;

class FaceEngineWrapper : public FaceEngine
{
    using FaceEngine::FaceEngine;

public:
    int initialize(const FaceEngineConfig& config) { return FaceEngine::initialize(config); }

    std::vector<Detection> detectFace(py::array_t<uint8_t> image)
    {
        py::buffer_info buf = image.request();
        if (buf.ndim != 3)
        {
            throw std::runtime_error("Invalid image shape");
        }
        ImageFormat format = ImageFormat::SBGR;
        if (buf.shape[2] == 4)
        {
            format = ImageFormat::SBGRA;
        }
        else if (buf.shape[2] == 1)
        {
            format = ImageFormat::GRAY8;
        }

        uint8_t* ptr1 = static_cast<uint8_t*>(buf.ptr);

        std::vector<Detection> detections;
        auto ret = FaceEngine::detectFace(ptr1, buf.shape[1], buf.shape[0], format, detections);
        if (ret != GFE_OK)
        {
            throw std::runtime_error("Detect face failed with error code: " + std::to_string(ret));
        }

        return detections;
    }

    FaceFeaturePacked extractFaceFeature(py::array_t<uint8_t> image, const Detection& det)
    {
        py::buffer_info buf = image.request();
        if (buf.ndim != 3)
        {
            throw std::runtime_error("Invalid image shape");
        }
        auto format = ImageFormat::SBGR;
        if (buf.shape[2] == 4)
        {
            format = ImageFormat::SBGRA;
        }
        else if (buf.shape[2] == 1)
        {
            format = ImageFormat::GRAY8;
        }

        uint8_t* ptr1 = static_cast<uint8_t*>(buf.ptr);
        FaceFeaturePacked packed;
        auto ret = FaceEngine::extractFaceFeature(ptr1, buf.shape[1], buf.shape[0], format, det, packed);
        if (ret != GFE_OK)
        {
            throw std::runtime_error("Extract feature failed with error code: " + std::to_string(ret));
        }

        return packed;
    }

    float compareFaceFeature(const FaceFeaturePacked& packed1, const FaceFeaturePacked& packed2)
    {
        return FaceEngine::compareFaceFeature(packed1, packed2);
    }
};

PYBIND11_MODULE(gawrs_face_py, m)
{
    py::class_<RelativeKeypoint>(m, "RelativeKeypoint")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def_property("x", &RelativeKeypoint::x, &RelativeKeypoint::setX)
        .def_property("y", &RelativeKeypoint::y, &RelativeKeypoint::setY);

    py::class_<AbsoluteKeypoint>(m, "AbsoluteKeypoint")
        .def(py::init<>())
        .def(py::init<int, int>())
        .def_property("x", &AbsoluteKeypoint::x, &AbsoluteKeypoint::setX)
        .def_property("y", &AbsoluteKeypoint::y, &AbsoluteKeypoint::setY);

    py::class_<RelativeBoundingBox>(m, "RelativeBoundingBox")
        .def(py::init<>())
        .def_readwrite("xmin", &RelativeBoundingBox::xmin)
        .def_readwrite("ymin", &RelativeBoundingBox::ymin)
        .def_readwrite("width", &RelativeBoundingBox::width)
        .def_readwrite("height", &RelativeBoundingBox::height);

    py::class_<AbsoluteBoundingBox>(m, "AbsoluteBoundingBox")
        .def(py::init<>())
        .def_readwrite("xmin", &AbsoluteBoundingBox::xmin)
        .def_readwrite("ymin", &AbsoluteBoundingBox::ymin)
        .def_readwrite("width", &AbsoluteBoundingBox::width)
        .def_readwrite("height", &AbsoluteBoundingBox::height);

    py::class_<BoundingBox>(m, "BoundingBox").def(py::init<>());

    py::class_<Detection>(m, "Detection")
        .def(py::init<>())
        .def_readwrite("label", &Detection::label)
        .def_readwrite("score", &Detection::score)
        .def_readonly("bounding_box", &Detection::boundingBox)
        .def_readwrite("keypoints", &Detection::keypoints);

    py::class_<FeatureVersion>(m, "FeatureVersion")
        .def(py::init<>())
        .def("__str__", [](const FeatureVersion& ver)
             { return fmt::format("FeatureVersion(major={}, minor={}, patch={})", ver.major, ver.minor, ver.patch); });

    py::class_<FaceFeaturePacked>(m, "FaceFeaturePacked")
        .def(py::init<>())
        .def_readonly("version", &FaceFeaturePacked::version)
        .def("__str__",
             [](const FaceFeaturePacked& packed)
             {
                 std::string ver =
                     fmt::format("{}.{}.{}", packed.version.major, packed.version.minor, packed.version.patch);

                 std::string s =
                     fmt::format("FaceFeaturePacked(version={}, feature_size={}, feature_type={}, bytes={})", ver,
                                 kFeatureSize, "float32", kPackedFeatureByteSize);
                 return s;
             });

    py::enum_<RotationModel>(m, "RotationModel")
        .value("ROTATE_0", RotationModel::GF_ROTATE_0)
        .value("ROTATE_90", RotationModel::GF_ROTATE_90)
        .value("ROTATE_180", RotationModel::GF_ROTATE_180)
        .value("ROTATE_270", RotationModel::GF_ROTATE_270)
        .value("HorizontalFlip", RotationModel::GF_HorizontalFlip)
        .value("VerticalFlip", RotationModel::GF_VerticalFlip)
        .export_values();

    py::enum_<RuntimeModel>(m, "RuntimeModel")
        .value("GF_NONE", RuntimeModel::GF_NONE)
        .value("GF_FACE_DETECTION", RuntimeModel::GF_FACE_DETECTION)
        .value("GF_FACE_RECOGNITION", RuntimeModel::GF_FACE_RECOGNITION)
        .export_values();

    py::enum_<GawrsFaceErrorCode>(m, "GawrsFaceErrorCode").export_values();

    py::class_<FaceEngineConfig>(m, "FaceEngineConfig")
        .def(py::init<>())
        .def_readwrite("detect_face_scale_val", &FaceEngineConfig::detectFaceScaleVal)
        .def_readwrite("detect_face_max_num", &FaceEngineConfig::detectFaceMaxNum)
        .def_readwrite("combined_mask", &FaceEngineConfig::combinedMask)
        .def_readwrite("prob_threshold", &FaceEngineConfig::probThreshold)
        .def_readwrite("nms_threshold", &FaceEngineConfig::nmsThreshold)
        .def_readwrite("rotation", &FaceEngineConfig::rotation)
        .def("set_support_face_detect", &FaceEngineConfig::setSupportFaceDetect)
        .def("is_support_face_detect", &FaceEngineConfig::isSupportFaceDetect)
        .def("set_support_face_recognize", &FaceEngineConfig::setSupportFaceRecognize)
        .def("is_support_face_recognize", &FaceEngineConfig::isSupportFaceRecognize);

    py::class_<FaceEngineWrapper>(m, "FaceEngine")
        .def(py::init<>())
        .def("initialize", &FaceEngineWrapper::initialize)
        .def("detect_face", &FaceEngineWrapper::detectFace)
        .def("extract_face_feature", &FaceEngineWrapper::extractFaceFeature)
        .def("compare_feature", &FaceEngineWrapper::compareFaceFeature);
}