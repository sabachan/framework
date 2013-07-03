#ifndef Math_NumericalUtils_H
#define Math_NumericalUtils_H

#include <Core/Cast.h>
#include <cmath>
#include <type_traits>

namespace sg {
//=============================================================================
namespace math {
namespace constant {
static double constexpr e         = 2.71828182845904523536;
static double constexpr log2e     = 1.44269504088896340736;
static double constexpr log10e    = 0.434294481903251827651;
static double constexpr ln2       = 0.693147180559945309417;
static double constexpr ln10      = 2.30258509299404568402;
static double constexpr PI        = 3.14159265358979323846;
static double constexpr PI_2      = 1.57079632679489661923;
static double constexpr PI_4      = 0.785398163397448309616;
static double constexpr _1_PI     = 0.318309886183790671538;
static double constexpr _2_PI     = 0.636619772367581343076;
static double constexpr _2_sqrtPI = 1.12837916709551257390;
static double constexpr sqrt2     = 1.41421356237309504880;
static double constexpr sqrt1_2   = 0.707106781186547524401;
}
}
//=============================================================================
template<typename T, typename U>
struct multiplication
{
    typedef decltype((*(T*)0) * (*(U*)0)) type;
};
//=============================================================================
template <typename T> inline T sq(T x) { return x*x; }
//=============================================================================
inline float floor(float x) { return std::floor(x); }
inline float round(float x) { return std::round(x); }
inline float ceil(float x) { return std::ceil(x); }
//=============================================================================
inline int floori(float x) { return int(checked_numcastable(std::floor(x))); }
inline int roundi(float x) { return int(checked_numcastable(std::round(x))); }
inline int ceili(float x) { return int(checked_numcastable(std::ceil(x))); }
//=============================================================================
template <typename T> inline typename std::enable_if<std::is_integral<T>::value, T>::type floori(T x) { return x; }
template <typename T> inline typename std::enable_if<std::is_integral<T>::value, T>::type roundi(T x) { return x; }
template <typename T> inline typename std::enable_if<std::is_integral<T>::value, T>::type ceili(T x)  { return x; }
//=============================================================================
template<typename T>
bool EqualsWithTolerance(T a, T b, T tolerance)
{
    using std::abs;
    return abs(b - a) <= tolerance;
}
//=============================================================================
namespace fast {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename U>
typename multiplication<T, U>::type
lerp(T a,
     typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
     U t)
{
    auto const r = a * U(1) + (b - a) * t;
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename U>
typename multiplication<T, U>::type
lerp(T y0,
     typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type y1,
     U x0,
     typename std::conditional<(sizeof(U)>sizeof(size_t)), U const&, U>::type x1,
     typename std::conditional<(sizeof(U)>sizeof(size_t)), U const&, U>::type x)
{
    auto const y = y0 * U(1) + (y1 - y0) * (x - x0) / (x1 - x0);
    return y;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
namespace symmetric {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename U>
typename multiplication<T, U>::type
lerp(T a,
     typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
     U t)
{
    auto const r = a * (U(1)-t) + b * t;
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename U>
typename multiplication<T, U>::type
lerp(T y0,
     typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type y1,
     U x0,
     typename std::conditional<(sizeof(U)>sizeof(size_t)), U const&, U>::type x1,
     typename std::conditional<(sizeof(U)>sizeof(size_t)), U const&, U>::type x)
{
    auto const y = (y0 * (x1 - x) + y1 * (x - x0)) / (x1 - x0);
    return y;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
using fast::lerp;
//=============================================================================
// Returns a value linearly interpolated from 0 to 1 when t spans from a to b.
// Outside of [a, b], the value is linearly extrapolated.
template<typename T, typename U = T>
U inverselerp(T a,
              typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
              typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type t)
{
    U const r = U(t - a) / U(b - a);
    return r;
}
//=============================================================================
// Clamps t to [a, b].
template<typename T>
T clamp(T t,
       typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type a,
       typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b)
{
    using std::max;
    using std::min;
    using componentwise::max;
    using componentwise::min;
    SG_ASSERT(min(a, b) == a);
    SG_ASSERT(max(a, b) == b);
    T const r = max(min(t, b), a);
    return r;
}
//=============================================================================
// Clamps t to [0, 1].
template<typename T>
T saturate(T t)
{
    return clamp(t, T(0), T(1));
}
//=============================================================================
template<typename T, typename U>
T quadraticBezier(T a,
                  typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
                  typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type c,
                  U t)
{
    U const u = U(1) - t;
    T const r = a * u*u + U(2) * b * t*u + c * t*t;
    return r;
}
//=============================================================================
template<typename T, typename U>
T cubicBezier(T a,
              typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
              typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type c,
              typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type d,
              U t)
{
    U const u = U(1) - t;
    U const t2 = t*t;
    U const u2 = u*u;
    T const r = a * u2*u + U(3) * b * t*u2 + U(3) * c * t2*u + d * t2*t;
    return r;
}
//=============================================================================
namespace tweening {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Returns a value that follows a parabolic arch, with following constraints:
//      input           0    1/2    1
//      output          0     1     0
//      derivative      4     0    -4
template<typename T>
T parabolicArc(T t)
{
    T const u = T(4) * t;
    T const r = u - u * t;
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Returns a value that follows a parabolic arch, with following constraints:
//      input           0     1
//      output          0     1
//      derivative      0     2
template<typename T>
T accelerate(T t)
{
    T const r = t * t;
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Returns a value that follows a parabolic arch, with following constraints:
//      input           0     1
//      output          0     1
//      derivative      2     0
template<typename T>
T decelerate(T t)
{
    T const r = T(2) * t - t * t;
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Returns a value linearly interpolated from 0 to 1 when t spans from a to b,
// clamped to [0, 1]
template<typename T, typename U = T>
U linearStep(T a,
             typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
             typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type t)
{
    U const u = clamp(t, T(a), T(b));
    U const r = U(u - a) / U(b - a);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, typename U = T>
U linearStep_AssumeInside(T a,
                          typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type b,
                          typename std::conditional<(sizeof(T)>sizeof(size_t)), T const&, T>::type t)
{
    SG_CODE_FOR_ASSERT(U const u = clamp(t, T(a), T(b));)
    SG_ASSERT(u == t);
    U const r = U(t - a) / U(b - a);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// Returns a value interpolated from 0 to 1 using a cubic Bezier curve when t
// spans from 0 to 1, clampted to [0, 1].
template<typename T>
T unitCubicStep(T t)
{
    T const u = clamp(t, T(0), T(1));
    T const u2 = u*u;
    T const r = u2 * (T(3) - T(2) * u);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
T unitCubicStep_AssumeInside(T t)
{
    SG_CODE_FOR_ASSERT(U const u = clamp(t, T(0), T(1));)
    SG_ASSERT(u == t);
    T const t2 = t*t;
    T const r = t2 * (T(3) - T(2) * t);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
}

#endif
