#pragma once
#include "vector.h"

namespace gawrs_face
{
template <class T>
class Point2
{
public:
    using ElementType = T;
    using Coords = Vector2<T>;

    Point2() : mData() {}
    Point2(const T& x, const T& y) : mX(x), mY(y) {}
    explicit Point2(const Coords& v) : mX(v.x()), mY(v.y()) {}

    Coords toVector() const { return Coords(mX, mY); }

    const T& operator[](std::size_t b) const { return mData[b]; }
    T& operator[](std::size_t b) { return mData[b]; }

public:
    const T& x() const { return mX; }
    const T& y() const { return mY; }

    void set(const T& x, const T& y)
    {
        mX = x;
        mY = y;
    }
    void setX(const T& x) { mX = x; }
    void setY(const T& y) { mY = y; }

private:
    union
    {
        struct
        {
            T mX;
            T mY;
        };

        T mData[2];
    };

    // Friend relational nonmember operators.
    friend bool operator==(const Point2& a, const Point2& b) { return a.mX == b.mX && a.mY == b.mY; }
    friend bool operator!=(const Point2& a, const Point2& b) { return a.mX != b.mX || a.mY != b.mY; }

    // Streaming operator.
    friend std::ostream& operator<<(std::ostream& out, const Point2& p)
    {
        return out << "(" << p.mData[0] << ", " << p.mData[1] << ")";
    }
};
} // namespace gawrs_face