#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iosfwd>
#include <limits>
#include <ostream>
#include <type_traits>

// reference: https://github.com/google/mediapipe/blob/master/mediapipe/framework/deps/vector.h

namespace gawrs_face
{
template <typename T>
class Vector2;
template <typename T>
class Vector3;
template <typename T>
class Vector4;
namespace internal
{
template <template <typename> class VecTemplate, typename T, std::size_t N>
class BasicVector
{
    using D = VecTemplate<T>;

protected:
    // Some methods return floating-point value even when T type is an integer.
    using FloatType = typename std::conditional<std::is_integral<T>::value, double, T>::type;
    using IdxSeqN = std::make_index_sequence<N>;

    template <std::size_t I, typename F, typename... As>
    static auto reduce(F f, As*... as) -> decltype(f(as[I]...))
    {
        return f(as[I]...);
    }

    template <typename R = D, std::size_t... Is, typename F, typename... As>
    static R generateEach(std::index_sequence<Is...>, F f, As*... as)
    {
        return R(reduce<Is>(f, as...)...);
    }

    // generate<R>(f,a,b,...) returns an R(...), where the constructor arguments
    // are created as a transform. R(f(a[0],b[0],...), f(a[1],b[1],...), ...),
    // and with a,b,...  all optional.
    template <typename R = D, typename F, typename... As>
    static R generate(F f, As&&... as)
    {
        return generateEach<R>(IdxSeqN(), f, std::forward<As>(as).data()...);
    }

public:
    enum
    {
        SIZE = N
    };

    static int kSize() { return SIZE; }

    // 索引
    T& operator[](int b) { return static_cast<D&>(*this).data()[b]; }
    T operator[](int b) const { return static_cast<const D&>(*this).data()[b]; }

    // 比较运算
    bool operator==(const D& b) const
    {
        const T* ap = static_cast<const D&>(*this).data();
        return std::equal(ap, ap + this->kSize(), b.data());
    }

    bool operator!=(const D& b) const { return !(asD() == b); }

    bool operator<(const D& b) const
    {
        const T* ap = static_cast<const D&>(*this).data();
        const T* bp = b.data();
        return std::lexicographical_compare(ap, ap + this->kSize(), bp, bp + b.kSize());
    }

    bool operator>(const D& b) const { return b < asD(); }
    bool operator<=(const D& b) const { return !(asD() > b); }
    bool operator>=(const D& b) const { return !(asD() < b); }

    D& operator+=(const D& b)
    {
        plusEq(static_cast<D&>(*this).data(), b.data(), IdxSeqN{});
        return static_cast<D&>(*this);
    }

    D& operator-=(const D& b)
    {
        minusEq(static_cast<D&>(*this).data(), b.data(), IdxSeqN{});
        return static_cast<D&>(*this);
    }

    D& operator*=(T k)
    {
        mulEq(static_cast<D&>(*this).data(), k, IdxSeqN{});
        return static_cast<D&>(*this);
    }

    D& operator/=(T k)
    {
        divEq(static_cast<D&>(*this).data(), k, IdxSeqN{});
        return static_cast<D&>(*this);
    }

    D operator+(const D& b) const { return D(asD()) += b; }
    D operator-(const D& b) const { return D(asD()) -= b; }
    D operator*(T k) const { return D(asD()) *= k; }
    D operator/(T k) const { return D(asD()) /= k; }

    friend D operator-(const D& a)
    {
        return generate([](const T& x) { return -x; }, a);
    }

    // Convert from another vector type
    template <typename T2>
    static D cast(const VecTemplate<T2>& b)
    {
        return generate([](const T2& x) { return static_cast<T>(x); }, b);
    }

    // multiply two vectors component by component
    D mulComponents(const D& b) const
    {
        return generate([](const T& x, const T& y) { return x * y; }, asD(), b);
    }
    // divide two vectors component by component
    D divComponents(const D& b) const
    {
        return generate([](const T& x, const T& y) { return x / y; }, asD(), b);
    }

    // Element-wise max.  {max(a[0],b[0]), max(a[1],b[1]), ...}
    friend D max(const D& a, const D& b)
    {
        return generate([](const T& x, const T& y) { return std::max(x, y); }, a, b);
    }

    // Element-wise min.  {min(a[0],b[0]), min(a[1],b[1]), ...}
    friend D min(const D& a, const D& b)
    {
        return generate([](const T& x, const T& y) { return std::min(x, y); }, a, b);
    }

    T dotProd(const D& b) const
    {
        return dot(static_cast<T>(0), static_cast<const D&>(*this).data(), b.data(), IdxSeqN{});
    }

    // Squared Euclidean norm (the dot product with itself).
    T norm2() const { return DotProd(asD()); }

    FloatType norm() const { return std::sqrt(norm2()); }

    // Normalized vector if the norm is nonzero. Not for integer types.
    D normalize() const
    {
        static_assert(!std::is_integral<T>::value, "must be floating point");
        T n = norm();
        if (n != T(0.0))
        {
            n = T(1.0) / n;
        }
        return D(asD()) *= n;
    }

    // Compose a vector from the sqrt of each component.
    D sqrt() const
    {
        return generate([](const T& x) { return std::sqrt(x); }, asD());
    }

    // Take the floor of each component.
    D floor() const
    {
        return generate([](const T& x) { return std::floor(x); }, asD());
    }

    // Take the ceil of each component.
    D ceil() const
    {
        return generate([](const T& x) { return std::ceil(x); }, asD());
    }

    // Round of each component.
    D fRound() const
    {
        return generate([](const T& x) { return std::rint(x); }, asD());
    }

    // True if any of the components is not a number.
    bool isNaN() const
    {
        bool r = false;
        const T* ap = asD().data();
        for (int i = 0; i < SIZE; ++i)
            r = r || std::isnan(ap[i]);
        return r;
    }

    // A Vector populated with all NaN values.
    static D nan()
    {
        return generate([] { return std::numeric_limits<T>::quiet_NaN(); });
    }

    friend std::ostream& operator<<(std::ostream& out, const D& v)
    {
        out << "[";
        const char* sep = "";
        for (int i = 0; i < SIZE; ++i)
        {
            out << sep;
            print(out, v[i]);
            sep = ", ";
        }
        return out << "]";
    }

    // These are only public for technical reasons.
    template <typename K>
    D mulScalarInternal(const K& k) const
    {
        return generate([k](const T& x) { return k * x; }, asD());
    }
    template <typename K>
    D divScalarInternal(const K& k) const
    {
        return generate([k](const T& x) { return k / x; }, asD());
    }

private:
    const D& asD() const { return static_cast<const D&>(*this); }
    D& asD() { return static_cast<D&>(*this); }

    // Ignores its arguments so that side-effects of variadic unpacking can occur.
    static void ignore(std::initializer_list<bool>) {}

    template <std::size_t... Is>
    static T dot(T sum, const T* a, const T* b, std::index_sequence<Is...>)
    {
        ignore({(sum += a[Is] * b[Is], true)...});
        return sum;
    }

    template <std::size_t... Is>
    static void plusEq(T* a, const T* b, std::index_sequence<Is...>)
    {
        ignore({(a[Is] += b[Is], true)...});
    }

    template <std::size_t... Is>
    static void minusEq(T* a, const T* b, std::index_sequence<Is...>)
    {
        ignore({(a[Is] -= b[Is], true)...});
    }

    template <std::size_t... Is>
    static void mulEq(T* a, T b, std::index_sequence<Is...>)
    {
        ignore({(a[Is] *= b, true)...});
    }

    template <std::size_t... Is>
    static void divEq(T* a, T b, std::index_sequence<Is...>)
    {
        ignore({(a[Is] /= b, true)...});
    }

    // ostream << uint8 prints the ASCII character, which is not useful.
    // Cast to int so that numbers will be printed instead.
    template <typename U>
    static void print(std::ostream& out, const U& v)
    {
        out << v;
    }
    static void print(std::ostream& out, uint8_t v) { out << static_cast<int>(v); }
};

// These templates must be defined outside of BasicVector so that the
// template specialization match algorithm must deduce 'a'.
template <typename K, template <typename> class VT2, typename T2, std::size_t N2>
VT2<T2> operator*(const K& k, const BasicVector<VT2, T2, N2>& a)
{
    return a.mulScalarInternal(k);
}
template <typename K, template <typename> class VT2, typename T2, std::size_t N2>
VT2<T2> operator/(const K& k, const BasicVector<VT2, T2, N2>& a)
{
    return a.divScalarInternal(k);
}
} // namespace internal

template <typename T>
class Vector2 : public internal::BasicVector<Vector2, T, 2>
{
    using Base = internal::BasicVector<Vector2, T, 2>;
    using VType = T;

public:
    typedef VType BaseType;
    using FloatType = typename Base::FloatType;
    using Base::SIZE;

public:
    Vector2() : mData() {}
    Vector2(T x, T y) : mData{x, y} {}

    explicit Vector2(const Vector3<T>& b) : Vector2(b.x(), b.y()) {}
    explicit Vector2(const Vector4<T>& b) : Vector2(b.x(), b.y()) {}

    bool aequal(const Vector2& vb, FloatType margin) const
    {
        return (std::fabs(mData[0] - vb.mData[0]) < margin) && (std::fabs(mData[1] - vb.mData[1]) < margin);
    }

    T crossProd(const Vector2& vb) const { return mData[0] * vb.mData[1] - mData[1] * vb.mData[0]; }

    FloatType angle(const Vector2& v) const { return std::atan2(crossProd(v), this->dotProd(v)); }

    Vector2 ortho() const { return Vector2(-mData[1], mData[0]); }

    Vector2 fabs() const { return Vector2(std::fabs(mData[0]), std::fabs(mData[1])); }
    Vector2 abs() const
    {
        static_assert(std::is_integral<VType>::value, "use Fabs for float_types");
        static_assert(static_cast<VType>(-1) == -1, "type must be signed");
        static_assert(sizeof(mData[0]) <= sizeof(int), "Abs truncates to int");
        return Vector2(std::abs(mData[0]), std::abs(mData[1]));
    }

public:
    T* data() { return mData; }
    const T* data() const { return mData; }
    void x(T v) { mData[0] = v; }
    void y(T v) { mData[1] = v; }
    T x() const { return mData[0]; }
    T y() const { return mData[1]; }

private:
    VType mData[SIZE];
};

template <typename T>
class Vector3 : public internal::BasicVector<Vector3, T, 3>
{
    using Base = internal::BasicVector<Vector3, T, 3>;
    using VType = T;

public:
    typedef VType BaseType;
    using FloatType = typename Base::FloatType;
    using Base::SIZE;

public:
    Vector3() : mData() {}
    Vector3(T x, T y, T z) : mData{x, y, z} {}

    Vector3(const Vector2<T>& b, T z) : Vector3(b.x(), b.y(), z) {}
    explicit Vector3(const Vector4<T>& b) : Vector3(b.x(), b.y(), b.z()) {}

    bool aequal(const Vector3& vb, FloatType margin) const
    {
        return (std::fabs(mData[0] - vb.mData[0]) < margin) && (std::fabs(mData[1] - vb.mData[1]) < margin) &&
               (std::fabs(mData[2] - vb.mData[2]) < margin);
    }

    Vector3 crossProd(const Vector3& vb) const
    {
        Vector3(mData[1] * vb.mData[2] - mData[2] * vb.mData[1], //
                mData[2] * vb.mData[0] - mData[0] * vb.mData[2], //
                mData[0] * vb.mData[1] - mData[1] * vb.mData[0]);
    }

    FloatType angle(const Vector3& va) const { return std::atan2(crossProd(va).norm(), this->dotProd(va)); }

    Vector3 ortho() const
    {
        int k = largestAbsComponent() - 1;
        if (k < 0)
            k = 2;
        Vector3 temp;
        temp[k] = T(1);
        return crossProd(temp).normalize();
    }

    Vector3 fabs() const { return abs(); }
    Vector3 abs() const
    {
        static_assert(!std::is_integral<VType>::value || static_cast<VType>(-1) == -1, "type must be signed");
        return Vector3(std::abs(mData[0]), std::abs(mData[1]), std::abs(mData[2]));
    }

    // return the index of the largest component (fabs)
    int largestAbsComponent() const
    {
        Vector3 temp = abs();
        return temp[0] > temp[1] ? temp[0] > temp[2] ? 0 : 2 : temp[1] > temp[2] ? 1 : 2;
    }

    // return the index of the smallest, median ,largest component of the vector
    Vector3<int> componentOrder() const
    {
        using std::swap;
        Vector3<int> temp(0, 1, 2);
        if (mData[temp[0]] > mData[temp[1]])
            swap(temp[0], temp[1]);
        if (mData[temp[1]] > mData[temp[2]])
            swap(temp[1], temp[2]);
        if (mData[temp[0]] > mData[temp[1]])
            swap(temp[0], temp[1]);
        return temp;
    }

public:
    T* data() { return mData; }
    const T* data() const { return mData; }
    void x(T v) { mData[0] = v; }
    void y(T v) { mData[1] = v; }
    void z(T v) { mData[2] = v; }
    T x() const { return mData[0]; }
    T y() const { return mData[1]; }
    T z() const { return mData[2]; }

private:
    VType mData[SIZE];
};

template <typename T>
class Vector4 : public internal::BasicVector<Vector4, T, 4>
{
    using Base = internal::BasicVector<Vector4, T, 4>;
    using VType = T;

public:
    typedef VType BaseType;
    using FloatType = typename Base::FloatType;
    using Base::SIZE;

public:
    Vector4() : mData() {}
    Vector4(T x, T y, T z, T w) : mData{x, y, z, w} {}

    Vector4(const Vector2<T>& a, T z, T w) : Vector4(a.x(), a.y(), z, w) {}
    Vector4(const Vector2<T>& a, const Vector2<T>& b) : Vector4(a.x(), a.y(), b.x(), b.y()) {}
    Vector4(const Vector3<T>& b, T w) : Vector4(b.x(), b.y(), b.z(), w) {}

    bool aequal(const Vector4& vb, FloatType margin) const
    {
        return (std::fabs(mData[0] - vb.mData[0]) < margin) && (std::fabs(mData[1] - vb.mData[1]) < margin) &&
               (std::fabs(mData[2] - vb.mData[2]) < margin) && (std::fabs(mData[3] - vb.mData[3]) < margin);
    }

    Vector4 fabs() const
    {
        return Vector4(std::fabs(mData[0]), std::fabs(mData[1]), std::fabs(mData[2]), std::fabs(mData[3]));
    }

    Vector4 abs() const
    {
        static_assert(std::is_integral<VType>::value, "use Fabs for float types");
        static_assert(static_cast<VType>(-1) == -1, "type must be signed");
        static_assert(sizeof(mData[0]) <= sizeof(int), "Abs truncates to int");
        return Vector4(std::abs(mData[0]), std::abs(mData[1]), std::abs(mData[2]), std::abs(mData[3]));
    }

public:
    T* data() { return mData; }
    const T* data() const { return mData; }
    void x(T v) { mData[0] = v; }
    void y(T v) { mData[1] = v; }
    void z(T v) { mData[2] = v; }
    void w(T v) { mData[3] = v; }
    T x() const { return mData[0]; }
    T y() const { return mData[1]; }
    T z() const { return mData[2]; }
    T w() const { return mData[3]; }

private:
    VType mData[SIZE];
};
} // namespace gawrs_face
