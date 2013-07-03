#ifndef Core_FixedPoint_H
#define Core_FixedPoint_H

#include "Assert.h"
#include <type_traits>

namespace sg {
//=============================================================================
namespace internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename U, int N, int order>
struct ShiftImpl
{
    static_assert(0 == order, "order must be -1, 0 or 1");
    T operator()(U iValue) { return iValue; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename U, int N>
struct ShiftImpl<T, U, N, 1> { T operator()(U iValue) { return iValue << N; } };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename U, int N>
struct ShiftImpl<T, U, N, -1> { T operator()(U iValue) { return iValue >> -N; } };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
template <typename T, typename U, int N>
T Shift(U iValue)
{
    typedef internal::ShiftImpl<T, U, N, (N == 0) ? 0 : (N > 0) ? 1 : -1> Impl;
    T value = Impl()(iValue);
    return value;
}
//=============================================================================
template <typename T, int N>
class FixedPoint
{
    static_assert(std::is_integral<T>::value, "T must be an integer type");
public:
    static int const bit_shift = N;
public:
    FixedPoint() : _(0) {}
    explicit FixedPoint(int iValue) : _(Shift<T,T,N>(iValue)) {}
    template <typename T2, int N2> explicit FixedPoint(FixedPoint<T2, N2> iValue) : _(Shift<T, T2, N-N2>(iValue._)) {}
    explicit FixedPoint(float iValue) : _(checked_numcastable(std::round(iValue * Shift<T,T,N>(1)))) {}

#define DEFINE_COMP_OP(OP) \
    friend bool operator OP (FixedPoint a, FixedPoint b) { return a._ OP b._; }

    DEFINE_COMP_OP(==)
    DEFINE_COMP_OP(!=)
    DEFINE_COMP_OP(<)
    DEFINE_COMP_OP(<=)
    DEFINE_COMP_OP(>=)
    DEFINE_COMP_OP(>)
#undef DEFINE_COMP_OP

    FixedPoint operator += (FixedPoint b) { this->_ += b._; return *this; }
    FixedPoint operator -= (FixedPoint b) { this->_ -= b._; return *this; }

    FixedPoint operator *= (T b) { this->_ *= b; return *this; }
    FixedPoint operator /= (T b) { this->_ /= b; return *this; }
    FixedPoint operator - () { FixedPoint r; r._ = -this->_; return r; }

    explicit operator T() const { return Shift<T,T,-N>(_); }
public:
    T _;
};
//=============================================================================
template <typename T, int N> FixedPoint<T, N> operator + (FixedPoint<T, N> a, FixedPoint<T, N> b) { FixedPoint<T, N> r; r._ = a._ + b._; return r; }
template <typename T, int N> FixedPoint<T, N> operator - (FixedPoint<T, N> a, FixedPoint<T, N> b) { FixedPoint<T, N> r; r._ = a._ - b._; return r; }
template <typename T, int N1, int N2> FixedPoint<T, N1+N2> operator * (FixedPoint<T, N1> a, FixedPoint<T, N2> b) { FixedPoint<T, N1+N2> r; r._ = a._ * b._; return r; }
template <typename T, int N> FixedPoint<T, N> operator * (FixedPoint<T, N> a, T b) { FixedPoint<T, N> r; r._ = a._ * b; return r; }
template <typename T, int N> FixedPoint<T, N> operator * (T a, FixedPoint<T, N> b) { FixedPoint<T, N> r; r._ = a * b._; return r; }
template <typename T, int N1, int N2> FixedPoint<T, N1-N2> operator / (FixedPoint<T, N1> a, FixedPoint<T, N2> b) { FixedPoint<T, N1-N2> r; r._ = a._ / b._; return r; }
template <typename T, int N> FixedPoint<T, N> operator / (FixedPoint<T, N> a, T b) { FixedPoint<T, N> r; r._ = a._ / b; return r; }
//=============================================================================
template <typename T, int N> FixedPoint<T, N> floor(FixedPoint<T, N> x) { x._ = x._ & ~(Shift<T,T,N>(1) - 1); return x; }
template <typename T, int N> FixedPoint<T, N> ceil (FixedPoint<T, N> x) { x._ = (x._ + Shift<T,T,N>(1) - 1) & ~(Shift<T,T,N>(1) - 1); return x; }
template <typename T, int N> FixedPoint<T, N> round(FixedPoint<T, N> x) { x._ = (x._ + Shift<T,T,N-1>(1) - 1) & ~(Shift<T,T,N>(1) - 1); return x; }
//=============================================================================
template <typename T, int N> T floori(FixedPoint<T, N> x) { return Shift<T,T,-N>(x._); }
template <typename T, int N> T ceili (FixedPoint<T, N> x) { return Shift<T,T,-N>(x._ + Shift<T,T,N>(1) - 1); }
template <typename T, int N> T roundi(FixedPoint<T, N> x) { return Shift<T,T,-N>(x._ + Shift<T,T,N-1>(1) - 1); }
//=============================================================================
}

#endif
