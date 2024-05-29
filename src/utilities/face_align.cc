#include <array>
#include <vector>

#include "gawrs_face/utilities/face_align.h"

namespace gawrs_face
{

std::array<float, 6> getAffineMatrix(std::array<float, 10> src, std::array<float, 10> dst)
{
    float ptmp[2];
    ptmp[0] = ptmp[1] = 0;
    for (int i = 0; i < 5; ++i)
    {
        ptmp[0] += src[i];
        ptmp[1] += src[5 + i];
    }
    ptmp[0] /= 5;
    ptmp[1] /= 5;
    for (int i = 0; i < 5; ++i)
    {
        src[i] -= ptmp[0];
        src[5 + i] -= ptmp[1];
        dst[i] -= ptmp[0];
        dst[5 + i] -= ptmp[1];
    }

    float dstX = (dst[3] + dst[4] - dst[0] - dst[1]) / 2, dstY = (dst[8] + dst[9] - dst[5] - dst[6]) / 2;
    float srcX = (src[3] + src[4] - src[0] - src[1]) / 2, srcY = (src[8] + src[9] - src[5] - src[6]) / 2;
    float theta = atan2(dstX, dstY) - atan2(srcX, srcY);

    float scale = sqrt(pow(dstX, 2) + pow(dstY, 2)) / sqrt(pow(srcX, 2) + pow(srcY, 2));
    float pts1[10];
    float pts0[2];
    float _a = sin(theta), _b = cos(theta);
    pts0[0] = pts0[1] = 0;
    for (int i = 0; i < 5; ++i)
    {
        pts1[i] = scale * (src[i] * _b + src[i + 5] * _a);
        pts1[i + 5] = scale * (-src[i] * _a + src[i + 5] * _b);
        pts0[0] += (dst[i] - pts1[i]);
        pts0[1] += (dst[i + 5] - pts1[i + 5]);
    }
    pts0[0] /= 5;
    pts0[1] /= 5;

    float sqloss = 0;
    for (int i = 0; i < 5; ++i)
    {
        sqloss += ((pts0[0] + pts1[i] - dst[i]) * (pts0[0] + pts1[i] - dst[i]) +
                   (pts0[1] + pts1[i + 5] - dst[i + 5]) * (pts0[1] + pts1[i + 5] - dst[i + 5]));
    }

    float square_sum = 0;
    for (float i : src)
    {
        square_sum += i * i;
    }
    for (int t = 0; t < 200; ++t)
    {
        _a = 0;
        _b = 0;
        for (int i = 0; i < 5; ++i)
        {
            _a += ((pts0[0] - dst[i]) * src[i + 5] - (pts0[1] - dst[i + 5]) * src[i]);
            _b += ((pts0[0] - dst[i]) * src[i] + (pts0[1] - dst[i + 5]) * src[i + 5]);
        }
        if (_b < 0)
        {
            _b = -_b;
            _a = -_a;
        }

        float _s = sqrt(_a * _a + _b * _b);
        _b /= _s;
        _a /= _s;
        for (int i = 0; i < 5; ++i)
        {
            pts1[i] = scale * (src[i] * _b + src[i + 5] * _a);
            pts1[i + 5] = scale * (-src[i] * _a + src[i + 5] * _b);
        }

        float _scale = 0;
        for (int i = 0; i < 5; ++i)
        {
            _scale += ((dst[i] - pts0[0]) * pts1[i] + (dst[i + 5] - pts0[1]) * pts1[i + 5]);
        }

        _scale /= (square_sum * scale);
        for (float& i : pts1)
        {
            i *= (_scale / scale);
        }
        scale = _scale;

        pts0[0] = pts0[1] = 0;
        for (int i = 0; i < 5; ++i)
        {
            pts0[0] += (dst[i] - pts1[i]);
            pts0[1] += (dst[i + 5] - pts1[i + 5]);
        }
        pts0[0] /= 5;
        pts0[1] /= 5;

        float _sqloss = 0;
        for (int i = 0; i < 5; ++i)
        {
            _sqloss += ((pts0[0] + pts1[i] - dst[i]) * (pts0[0] + pts1[i] - dst[i]) +
                        (pts0[1] + pts1[i + 5] - dst[i + 5]) * (pts0[1] + pts1[i + 5] - dst[i + 5]));
        }
        if (abs(_sqloss - sqloss) < 1e-2)
        {
            break;
        }
        sqloss = _sqloss;
    }

    for (int i = 0; i < 5; ++i)
    {
        pts1[i] += (pts0[0] + ptmp[0]);
        pts1[i + 5] += (pts0[1] + ptmp[1]);
    }

    float m0 = _b * scale;
    float m1 = _a * scale;
    float m3 = -_a * scale;
    float m4 = _b * scale;
    float m2 = pts0[0] + ptmp[0] - scale * (ptmp[0] * _b + ptmp[1] * _a);
    float m5 = pts0[1] + ptmp[1] - scale * (-ptmp[0] * _a + ptmp[1] * _b);

    return {m0, m1, m2, m3, m4, m5};
}

void warpAffineMatrix(const std::vector<unsigned char>& srcU, std::vector<unsigned char>& dstU, //
                      std::array<float, 6> m, int srcW, int srcH, int dstW, int dstH)
{
    float D = m[0] * m[4] - m[1] * m[3];
    D = D != 0 ? (float)1. / D : 0;
    float A11 = m[4] * D, A22 = m[0] * D;
    m[0] = A11;
    m[1] *= -D;
    m[3] *= -D;
    m[4] = A22;
    float b1 = -m[0] * m[2] - m[1] * m[5];
    float b2 = -m[3] * m[2] - m[4] * m[5];
    m[2] = b1;
    m[5] = b2;

    for (int y = 0; y < dstH; y++)
    {
        for (int x = 0; x < dstW; x++)
        {
            float fx = m[0] * (float)x + m[1] * (float)y + m[2];
            float fy = m[3] * (float)x + m[4] * (float)y + m[5];

            int sy = (int)floor(fy);
            fy -= (float)sy;
            if (sy < 0 || sy >= srcH)
                continue;

            short cbufy[2];
            cbufy[0] = (short)((1.f - fy) * 2048);
            cbufy[1] = short(2048 - cbufy[0]);

            int sx = (int)floor(fx);
            fx -= (float)sx;
            if (sx < 0 || sx >= srcW)
                continue;

            short cbufx[2];
            cbufx[0] = (short)((1.f - fx) * 2048);
            cbufx[1] = short(2048 - cbufx[0]);

            if (sy == srcH - 1 || sx == srcW - 1)
                continue;
            for (int c = 0; c < 3; c++)
            {
                dstU[3 * (y * dstW + x) + c] = (srcU[3 * (sy * srcW + sx) + c] * cbufx[0] * cbufy[0] +
                                                srcU[3 * ((sy + 1) * srcW + sx) + c] * cbufx[0] * cbufy[1] +
                                                srcU[3 * (sy * srcW + sx + 1) + c] * cbufx[1] * cbufy[0] +
                                                srcU[3 * ((sy + 1) * srcW + sx + 1) + c] * cbufx[1] * cbufy[1]) >>
                                               22;
            }
        }
    }
}

ncnn::Mat normCrop(const ncnn::Mat& inImage, const Detection& det, int outWidth, int outHeight)
{
    auto kps = std::get<RelativeKeypoints>(det.keypoints);
    std::array<float, 10> srcPs;
    int lmkSize = std::min(5, (int)kps.size());
    for (int i = 0; i < lmkSize; ++i)
    {
        srcPs[i] = kps[i].x();
        srcPs[i + 5] = kps[i].y();
    }
    std::array<float, 10> dstPs{38.2946, 73.5318, 56.0252, 41.5493, 70.7299,
                                51.6963, 51.5014, 71.7366, 92.3655, 92.2041};

    int srcW = inImage.w;
    int srcH = inImage.h;
    int dstW = outWidth;
    int dstH = outHeight;
    std::vector<unsigned char> srcU(srcW * srcH * 3), dstU(dstW * dstH * 3);
    inImage.to_pixels(srcU.data(), ncnn::Mat::PIXEL_RGB);

    auto M = getAffineMatrix(srcPs, dstPs);
    warpAffineMatrix(srcU, dstU, M, srcW, srcH, dstW, dstH);
    auto outImage = ncnn::Mat::from_pixels(dstU.data(), ncnn::Mat::PIXEL_RGB, dstW, dstH);

    return outImage;
}

} // namespace gawrs_face