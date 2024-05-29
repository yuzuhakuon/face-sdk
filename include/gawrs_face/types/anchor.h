
namespace gawrs_face
{
struct Anchor
{
    float xCenter;
    float yCenter;
    float width;
    float height;

    static Anchor fromTLBR(float top, float left, float bottom, float right)
    {
        Anchor a;
        a.xCenter = (left + right) / 2.0f;
        a.yCenter = (top + bottom) / 2.0f;
        a.width = right - left;
        a.height = bottom - top;
        return a;
    }

    static Anchor fromXYWH(float x, float y, float w, float h)
    {
        Anchor a;
        a.xCenter = x + w / 2.0f;
        a.yCenter = y + h / 2.0f;
        a.width = w;
        a.height = h;
        return a;
    }
};
} // namespace gawrs_face