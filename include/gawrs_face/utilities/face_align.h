#pragma once
#include <mat.h>

#include "gawrs_face/types/detection.h"

namespace gawrs_face
{
ncnn::Mat normCrop(const ncnn::Mat& inImage, const Detection& det, int outWidth, int outHeight);
}