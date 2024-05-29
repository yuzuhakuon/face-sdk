#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <ostream>

#include "point2.h"

// reference: https://github.com/google/mediapipe/blob/master/mediapipe/framework/deps/rectangle.h

namespace gawrs_face
{
template <typename T>
class Rectangle;

template <typename T>
std::ostream& operator<<(std::ostream&, const Rectangle<T>&);

template <class T>
class Rectangle
{
public:
    using Self = Rectangle<T>;

    // Default constructed rectangle which is empty.
    Rectangle() { setEmpty(); }

    // Creates a rectangle from the minimum point and the dimensions.
    Rectangle(const T& x, const T& y, const T& width, const T& height);

    Rectangle(const Point2<T>& p0, const Point2<T>& p1);

    // Sets min to be very large numbers and max to be very large negative numbers
    // so that points can be used to correctly extend the rectangle.
    void setEmpty();

    // A rectangle is empty if there are no points inside of it.  A degenerate
    // rectangle where the corners are coincident has zero area but is not empty.
    bool isEmpty() const { return mMin.x() > mMax.x() || mMin.y() > mMax.y(); }

    // Width and height are both max - min, which may be negative if SetEmpty()
    // was called or the user explicity set the min and max points.
    T width() const { return mMax.x() - mMin.x(); }
    T height() const { return mMax.y() - mMin.y(); }

    // Computes the area, which is negative if the width xor height is negative.
    // The value is undefined if SetEmpty() is called.
    // Watch out for large integer rectangles because the area may overflow.
    T area() const { return width() * height(); }

    // Accessors are provided for both points and sides.
    const T& xmin() const { return mMin.x(); }
    const T& xmax() const { return mMax.x(); }
    const T& ymin() const { return mMin.y(); }
    const T& ymax() const { return mMax.y(); }

    // Returns the min and max corner points.
    const Point2<T>& minXY() const { return mMin; }
    const Point2<T>& maxXY() const { return mMax; }

    bool operator==(const Rectangle&) const;
    bool operator!=(const Rectangle&) const;

    // Sets the geometry of the rectangle given two points.
    // The resulting rectangle will have non-negative width and height.
    void set(const Point2<T>& p0, const Point2<T>& p1);

    // Sets the geometry of the rectangle given a minimum point and dimensions.
    void set(const T& x, const T& y, const T& width, const T& height);

    // Sets the min and max values, and min greater than max is allowable,
    // but the user has to be aware of the consequences such as negative width
    // and height.  Both point and side accessors are provided.
    void setXMin(const T& x) { mMin.setX(x); }
    void setXMax(const T& x) { mMax.setX(x); }
    void setYMin(const T& y) { mMin.setY(y); }
    void setYMax(const T& y) { mMax.setY(y); }

    void setMinXY(const Point2<T>& p) { mMin.set(p.x(), p.y()); }
    void setMaxXY(const Point2<T>& p) { mMax.set(p.x(), p.y()); }

    // Expands a rectangle to contain a point or vector.
    void expand(const T& x, const T& y);
    void expand(const Point2<T>& p);
    // Expands a rectangle to contain another rectangle.
    void expand(const Rectangle& other);

    // Returns the union of this rectangle with another rectangle, which
    // is the smallest rectangle that contains both rectangles.
    Rectangle unionArea(const Rectangle& other) const;

    // Returns the intersection of this rectangle with another rectangle.
    // If the intersection is empty, returns a rectangle initialized by
    // SetEmpty().
    Rectangle intersect(const Rectangle& other) const;

    // Tests if this rectangle has a non-empty intersection with another rectangle
    // including the boundary.
    bool intersects(const Rectangle& other) const;

    // Tests if a point is inside or on any of the 4 edges of the rectangle.
    bool contains(const T& x, const T& y) const;
    bool contains(const Point2<T>& pt) const;

    // Tests if a rectangle is inside or on any of the 4 edges of the rectangle.
    bool contains(const Rectangle& other) const;

    // Adds a border around the rectangle by subtracting the border size from the
    // min point and adding it to the max point.  The border size can be
    // negative.
    void addBorder(const T& border_size);

private:
    Point2<T> mMin;
    Point2<T> mMax;
};

template <typename T>
Rectangle<T>::Rectangle(const Point2<T>& p0, const Point2<T>& p1)
{
    set(p0, p1);
}

template <typename T>
Rectangle<T>::Rectangle(const T& x, const T& y, const T& width, const T& height)
{
    set(x, y, width, height);
}

// The general version works only when T models Integer (there are more
// integer classes than float classes).
template <typename T>
void Rectangle<T>::setEmpty()
{
    T min_value = std::numeric_limits<T>::min();
    T max_value = std::numeric_limits<T>::max();
    mMin.set(max_value, max_value);
    mMax.set(min_value, min_value);
}

template <>
inline void Rectangle<float>::setEmpty()
{
    float max_value = std::numeric_limits<float>::max();
    mMin.set(max_value, max_value);
    mMax.set(-max_value, -max_value);
}

template <>
inline void Rectangle<double>::setEmpty()
{
    double max_value = std::numeric_limits<double>::max();
    mMin.set(max_value, max_value);
    mMax.set(-max_value, -max_value);
}

template <typename T>
bool Rectangle<T>::operator==(const Rectangle<T>& other) const
{
    return mMin == other.mMin && mMax == other.mMax;
}

template <typename T>
bool Rectangle<T>::operator!=(const Rectangle<T>& other) const
{
    return !(*this == other);
}

template <typename T>
void Rectangle<T>::set(const Point2<T>& p0, const Point2<T>& p1)
{
    if (p0.x() <= p1.x())
        mMin.setX(p0.x()), mMax.setX(p1.x());
    else
        mMax.setX(p0.x()), mMin.setX(p1.x());

    if (p0.y() <= p1.y())
        mMin.setY(p0.y()), mMax.setY(p1.y());
    else
        mMax.setY(p0.y()), mMin.setY(p1.y());
}

template <typename T>
void Rectangle<T>::set(const T& x, const T& y, const T& width, const T& height)
{
    mMin.set(x, y);
    mMax.set(x + width, y + height);
}

template <typename T>
void Rectangle<T>::expand(const T& x, const T& y)
{
    mMin.set(std::min(x, xmin()), std::min(y, ymin()));
    mMax.set(std::max(x, xmax()), std::max(y, ymax()));
}

template <typename T>
void Rectangle<T>::expand(const Point2<T>& p)
{
    expand(p.x(), p.y());
}

template <typename T>
void Rectangle<T>::expand(const Rectangle<T>& other)
{
    expand(other.min_);
    expand(other.max_);
}

template <typename T>
bool Rectangle<T>::contains(const T& x, const T& y) const
{
    return x >= xmin() && x <= xmax() && y >= ymin() && y <= ymax();
}

template <typename T>
bool Rectangle<T>::contains(const Point2<T>& p) const
{
    return contains(p.x(), p.y());
}

template <typename T>
bool Rectangle<T>::contains(const Rectangle<T>& r) const
{
    return Contains(r.mMin) && Contains(r.mMax);
}

template <typename T>
Rectangle<T> Rectangle<T>::unionArea(const Rectangle<T>& r) const
{
    return Rectangle<T>(Point2<T>(std::min(xmin(), r.xmin()), std::min(ymin(), r.ymin())),
                        Point2<T>(std::max(xmax(), r.xmax()), std::max(ymax(), r.ymax())));
}

template <typename T>
Rectangle<T> Rectangle<T>::intersect(const Rectangle<T>& r) const
{
    Point2<T> pmin(std::max(xmin(), r.xmin()), std::max(ymin(), r.ymin()));
    Point2<T> pmax(std::min(xmax(), r.xmax()), std::min(ymax(), r.ymax()));

    if (pmin.x() > pmax.x() || pmin.y() > pmax.y())
        return Rectangle<T>();
    else
        return Rectangle<T>(pmin, pmax);
}

template <typename T>
bool Rectangle<T>::intersects(const Rectangle<T>& r) const
{
    return !(isEmpty() || r.isEmpty() || r.xmax() < xmin() || xmax() < r.xmin() || r.ymax() < ymin() ||
             ymax() < r.ymin());
}

template <typename T>
void Rectangle<T>::addBorder(const T& borderSize)
{
    mMin.set(xmin() - borderSize, ymin() - borderSize);
    mMax.set(xmax() + borderSize, ymax() + borderSize);
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const Rectangle<T>& r)
{
    out << "[(" << r.xmin() << ", " << r.ymin() << "), (" << r.xmax() << ", " << r.ymax() << ")]";
    return out;
}

using RectangleF = Rectangle<float>;
using RectangleI = Rectangle<int32_t>;
} // namespace gawrs_face