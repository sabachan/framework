#ifndef Math_Vector_H
#define Math_Vector_H

#include "NumericalUtils.h"
#include <Core/Config.h>
#include <Core/For.h>
#include <Core/IntTypes.h>
#include <Core/TemplateUtils.h>
#include <Core/Utils.h>
#include <algorithm>
#include <limits>
#include <type_traits>

namespace sg {
namespace math {
//=============================================================================
template<typename T, size_t dim>
class Vector;
//=============================================================================
template <typename T, size_t dim, size_t sub_dim, size_t _0, size_t _1, size_t _2 = -1, size_t _3 = -1>
class Vector_SwizzledSubVector
{
    Vector_SwizzledSubVector& operator= (Vector_SwizzledSubVector const&); // delete
public:
    static_assert(2 <= sub_dim, "Vector::SwizzledSubVector must be at least 2 dimensionnal.");
    static_assert(4 >= sub_dim, "Vector::SwizzledSubVector must be at most 4 dimensionnal.");
    static_assert(_0 < dim, "");
    static_assert(_1 < dim, "");
    static_assert(_2 < dim || (sub_dim <= 2 && _2 == -1), "");
    static_assert(_3 < dim || (sub_dim <= 3 && _3 == -1), "");
    static bool constexpr is_injectif =
        (_0 != _1 || -1 == _0 || -1 == _1) &&
        (_0 != _2 || -1 == _0 || -1 == _2) &&
        (_0 != _3 || -1 == _0 || -1 == _3) &&
        (_1 != _2 || -1 == _1 || -1 == _2) &&
        (_1 != _3 || -1 == _1 || -1 == _3) &&
        (_2 != _3 || -1 == _2 || -1 == _3);
    typedef Vector<T, dim> vector_type;
    typedef T value_type;
public:
    Vector_SwizzledSubVector(vector_type& ref): m_ref(ref) {}
    Vector_SwizzledSubVector& operator=(Vector<value_type, sub_dim> const& iOther)
    {
        static_assert(is_injectif, "You cannot assign to a non injective swizzling");
        if(SG_CONSTANT_CONDITION(sub_dim >= 1)) { SG_ASSERT(_0 < dim); m_ref._[_0] = iOther._[0]; }
        if(SG_CONSTANT_CONDITION(sub_dim >= 2)) { SG_ASSERT(_1 < dim); m_ref._[_1] = iOther._[1]; }
        if(SG_CONSTANT_CONDITION(sub_dim >= 3)) { SG_ASSERT(_2 < dim); m_ref._[_2] = iOther._[2]; }
        if(SG_CONSTANT_CONDITION(sub_dim >= 4)) { SG_ASSERT(_3 < dim); m_ref._[_3] = iOther._[3]; }
        return *this;
    }

private:
    template<typename, size_t> friend class Vector;
    vector_type& m_ref;
};
//=============================================================================
template<typename T, size_t dim>
class Vector
{
    static_assert(1 <= dim, "Vector must be at least 1 dimensionnal.");
public:
    typedef Vector<T, dim> this_type;
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
    struct CompResult : public Vector<bool, dim>
    {
        friend bool all(CompResult const& r) { for_range(size_t, i, 0, dim) { if(!r._[i]) return false; } return true; }
        friend bool any(CompResult const& r) { for_range(size_t, i, 0, dim) { if( r._[i]) return true; } return false; }
        CompResult operator!() { CompResult r; for_range(size_t, i, 0, dim) { r._[i] = !this->_[i]; } return r; }
    };
    struct CompResultDefaultAny;
    struct CompResultDefaultAll : public CompResult
    {
        operator bool() const { return all(*this); };
        Vector<T, dim>::CompResultDefaultAny operator!() { Vector<T, dim>::CompResultDefaultAny r; for_range(size_t, i, 0, dim) { r._[i] = !this->_[i]; } return r; }
    };
    struct CompResultDefaultAny : public CompResult
    {
        operator bool() const { return any(*this); };
        Vector<T, dim>::CompResultDefaultAll operator!() { Vector<T, dim>::CompResultDefaultAll r; for_range(size_t, i, 0, dim) { r._[i] = !this->_[i]; } return r; }
    };
public:
    explicit Vector(uninitialized_t) {}
    Vector() { for(size_t i = 0; i < dim; ++i) { _[i] = value_type(); } }
    explicit Vector(value_type_for_const_passing broadcasted) {  for(size_t i = 0; i < dim; ++i) { _[i] = broadcasted; } }
    // TODO (dès que C++11 est disponible): Vector(std::initializer_list);
    Vector(value_type_for_const_passing x, value_type_for_const_passing y) { static_assert(2 == dim, "incorrect number of constructor parameters."); _[0] = x; _[1] = y; }
    Vector(value_type_for_const_passing x, value_type_for_const_passing y, value_type_for_const_passing z) { static_assert(3 == dim, "incorrect number of constructor parameters."); _[0] = x; _[1] = y; _[2] = z; }
    Vector(value_type_for_const_passing x, value_type_for_const_passing y, value_type_for_const_passing z, value_type_for_const_passing w) { static_assert(4 == dim, "incorrect number of constructor parameters."); _[0] = x; _[1] = y; _[2] = z; _[3] = w; }
    Vector(value_type const(& v)[dim]) { for(size_t i = 0; i < dim; ++i) { _[i] = v[i]; } }
    Vector(Vector const& iOther);
    Vector & operator=(Vector const& iOther);
    template<typename T2> explicit Vector(Vector<T2, dim> const& iOther) { for(size_t i = 0; i < dim; ++i) { _[i] = T(iOther._[i]); } }
    template<size_t other_dim, size_t _0, size_t _1, size_t _2, size_t _3>
    Vector(Vector_SwizzledSubVector<T, other_dim, dim, _0, _1, _2, _3> const& iOther)
    {
        if(SG_CONSTANT_CONDITION(dim >= 1)) { _[0] = iOther.m_ref._[_0]; }
        if(SG_CONSTANT_CONDITION(dim >= 2)) { _[1] = iOther.m_ref._[_1]; }
        if(SG_CONSTANT_CONDITION(dim >= 3)) { _[2] = iOther.m_ref._[_2]; }
        if(SG_CONSTANT_CONDITION(dim >= 4)) { _[3] = iOther.m_ref._[_3]; }
    }

    value_type_for_const_passing operator[](size_t i) const { SG_ASSERT(i < dim); return _[i]; }
    value_type&                  operator[](size_t i)       { SG_ASSERT(i < dim); return _[i]; }
    value_type_for_const_passing x() const { static_assert(1 <= dim, "x not available."); return _[0]; }
    value_type_for_const_passing y() const { static_assert(2 <= dim, "y not available."); return _[1]; }
    value_type_for_const_passing z() const { static_assert(3 <= dim, "z not available."); return _[2]; }
    value_type_for_const_passing w() const { static_assert(4 <= dim, "w not available."); return _[3]; }
    value_type&                  x()       { static_assert(1 <= dim, "x not available."); return _[0]; }
    value_type&                  y()       { static_assert(2 <= dim, "y not available."); return _[1]; }
    value_type&                  z()       { static_assert(3 <= dim, "z not available."); return _[2]; }
    value_type&                  w()       { static_assert(4 <= dim, "w not available."); return _[3]; }

    value_type LengthSq() const { value_type lsq = value_type(); for(size_t i = 0; i < dim; ++i) { lsq += _[i] * _[i]; } return lsq; }
    Vector Normalised() const;

    template<size_t dim2> Vector<T, dim2> SubVector(size_t i) const;
    template<size_t dim2> void SetSubVector(size_t i, Vector<T, dim2> const& iSubVector);
    Vector<T, dim + 1> Append(value_type_for_const_passing b) const;
    template<size_t dim2> Vector<T, dim + dim2> Append(Vector<T, dim2> const& b) const;

#define DEFINE_COMPONENTWISE_OP(OP) \
    Vector const& operator OP ## = (Vector const& a) \
    { \
        for(size_t i = 0; i < dim; ++i) { _[i] OP ## = a._[i]; } \
        return *this; \
    } \
    Vector const& operator OP ## = (value_type_for_const_passing s) \
    { \
        for(size_t i = 0; i < dim; ++i) { _[i] OP ## = s; } \
        return *this; \
    } \
    /* Note that a is taken by copy */ \
    /*  (cf. http://stackoverflow.com/questions/4421706/operator-overloading/4421729#4421729) */ \
    friend Vector operator OP (Vector a, Vector const& b) \
    { \
        return a OP ## = b; \
    } \
    friend Vector operator OP (Vector a, value_type_for_const_passing s) \
    { \
        return a OP ## = s; \
    } \
    friend Vector operator OP (value_type_for_const_passing s, Vector a) \
    { \
        for(size_t i = 0; i < dim; ++i) { a._[i] = s OP a._[i]; } \
        return a; \
    }

    // NB: it is possible to write vetor + scalar, and scalar will be brodcasted.
    // This is the same behavior as in DX shaders.
    DEFINE_COMPONENTWISE_OP(+)
    DEFINE_COMPONENTWISE_OP(-)
    DEFINE_COMPONENTWISE_OP(*)
    DEFINE_COMPONENTWISE_OP(/)
    DEFINE_COMPONENTWISE_OP(%)
    DEFINE_COMPONENTWISE_OP(&)
    DEFINE_COMPONENTWISE_OP(|)
    DEFINE_COMPONENTWISE_OP(>>)
    DEFINE_COMPONENTWISE_OP(<<)
#undef DEFINE_COMPONENTWISE_OP

#define RETURN_VECTOR_TYPE typename std::enable_if<dim != 1 && std::is_arithmetic<S>::value, Vector<decltype(std::declval<value_type>() * std::declval<S>()), dim> >::type
#define DEFINE_MIXED_COMPONENTWISE_OP(OP) \
    template <typename S> \
    friend RETURN_VECTOR_TYPE operator OP (Vector const& a, Vector<S, dim> const& b) \
    { \
        RETURN_VECTOR_TYPE r; \
        for(size_t i = 0; i < dim; ++i) { r._[i] = a._[i] OP b._[i]; } \
        return r; \
    } \
    template <typename S> \
    friend RETURN_VECTOR_TYPE operator OP (Vector a, S s) \
    { \
        RETURN_VECTOR_TYPE r; \
        for(size_t i = 0; i < dim; ++i) { r._[i] = a._[i] OP s; } \
        return r; \
    } \
    template <typename S> \
    friend RETURN_VECTOR_TYPE operator OP (S s, Vector a) \
    { \
        RETURN_VECTOR_TYPE r; \
        for(size_t i = 0; i < dim; ++i) { r._[i] = s OP a._[i]; } \
        return r; \
    }

    DEFINE_MIXED_COMPONENTWISE_OP(+)
    DEFINE_MIXED_COMPONENTWISE_OP(-)
    DEFINE_MIXED_COMPONENTWISE_OP(*)
    DEFINE_MIXED_COMPONENTWISE_OP(/)
#undef DEFINE_MIXED_COMPONENTWISE_OP
#undef RETURN_VECTOR_TYPE

#define DEFINE_COMPONENTWISE_UNARY_OP(OP) \
    Vector operator OP () const \
    { \
        Vector r = Vector(uninitialized); \
        for(size_t i = 0; i < dim; ++i) { r._[i] = OP _[i]; } \
        return r; \
    }

    DEFINE_COMPONENTWISE_UNARY_OP(-)
#undef DEFINE_COMPONENTWISE_UNARY_OP

#define Math_Vector_Include_SwizzleImpl
#include "Vector_SwizzleImpl.h"
#undef Math_Vector_Include_SwizzleImpl
public:
    // Q: Why a public member, moreover with such name as "_" ?
    // A: It allows fast access in debug, without function call nor check,
    //    by using construct such as v._[i] instead of v[i].
    //    Moreover, exposing it publicly allows this type to be used with
    //    reflection mechanisms.
    value_type _[dim];
};
//=============================================================================
template<typename T, size_t dim>
Vector<T, dim>::Vector(Vector const& iOther)
{
    SG_ASSUME(this != &iOther);
    for(size_t i = 0; i < dim; ++i)
    {
        _[i] = iOther._[i];
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
Vector<T, dim>& Vector<T, dim>::operator=(Vector const& iOther)
{
    for(size_t i = 0; i < dim; ++i)
    {
        _[i] = iOther._[i];
    }
    return *this;
}
//=============================================================================
template<typename T, size_t dim>
Vector<T, dim> Vector<T, dim>::Normalised() const
{
     // Maybe add support for complex or other ?
    static_assert(std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_integer, "T must be a floating point type");
    T const normsq = LengthSq();
    SG_ASSERT(0 < normsq);
    T const oonorm = 1.f / sqrt(normsq); // WARNING!
    Vector<T, dim> r = Vector<T, dim>(uninitialized);
    for(size_t i = 0; i < dim; ++i)
    {
        r._[i] = _[i] * oonorm;
    }
    return r;
}
//=============================================================================
template<typename T, size_t dim>
template<size_t dim2>
Vector<T, dim2> Vector<T, dim>::SubVector(size_t i) const
{
    static_assert(dim2 <= dim, "");
    SG_ASSERT(i + dim2 <= dim);
    Vector<T, dim2> subVec;
    for(size_t ki = 0; ki < dim2; ++ki)
    {
        subVec._[ki] = _[ki+i];
    }
    return subVec;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
template<size_t dim2>
void Vector<T, dim>::SetSubVector(size_t i, Vector<T, dim2> const& iSubVector)
{
    static_assert(dim2 <= dim, "");
    SG_ASSERT(i + dim2 <= dim);
    for(size_t ki = 0; ki < dim2; ++ki)
    {
        _[ki+i] = iSubVector._[ki];
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
Vector<T, dim + 1>  Vector<T, dim>::Append(value_type_for_const_passing b) const
{
    Vector<T, dim + 1> r(uninitialized);
    r.SetSubVector(0, *this);
    r._[dim] = b;
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
template<size_t dim2>
Vector<T, dim + dim2>  Vector<T, dim>::Append(Vector<T, dim2> const& b) const
{
    Vector<T, dim + dim2> r(uninitialized);
    r.SetSubVector(0, *this);
    r.SetSubVector(dim, b);
    return r;
}
//=============================================================================
#if 1
template<typename T, size_t dim>
typename Vector<T, dim>::CompResultDefaultAll operator== (Vector<T, dim> const& a, Vector<T, dim> const& b)
{
    typename Vector<T, dim>::CompResultDefaultAll r;
    for(size_t i = 0; i < dim; ++i)
    {
        r._[i] = a._[i] == b._[i];
    }
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
typename Vector<T, dim>::CompResultDefaultAny operator!= (Vector<T, dim> const& a, Vector<T, dim> const& b)
{
    return !(a == b);
}
#else
//=============================================================================
template<typename T, size_t dim>
bool operator== (Vector<T, dim> const& a, Vector<T, dim> const& b)
{
    for(size_t i = 0; i < dim; ++i)
    {
        if (a._[i] != b._[i]) return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
bool operator!= (Vector<T, dim> const& a, Vector<T, dim> const& b)
{
    return !(a == b);
}
#endif
//=============================================================================
template<typename T, size_t dim>
bool EqualsWithTolerance(Vector<T, dim> const& a, Vector<T, dim> const& b, Vector<T, dim> const& tolerance)
{
    return AllLessEqual(componentwise::abs(b - a), tolerance);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
bool EqualsWithTolerance(Vector<T, dim> const& a, Vector<T, dim> const& b, T tolerance)
{
    return EqualsWithTolerance(a, b, Vector<T, dim>(tolerance));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim, size_t dim0, size_t _0, size_t _1, size_t _2, size_t _3, typename U>
bool EqualsWithTolerance (Vector<T, dim> const& a, Vector_SwizzledSubVector<T, dim0, dim, _0, _1, _2, _3> const& b, U tolerance)
{
    return EqualsWithTolerance(a, Vector<T, dim>(b), tolerance);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim, size_t dim0, size_t _0, size_t _1, size_t _2, size_t _3, typename U>
bool EqualsWithTolerance (Vector_SwizzledSubVector<T, dim0, dim, _0, _1, _2, _3> const& a, Vector<T, dim> const& b, U tolerance)
{
    return EqualsWithTolerance(Vector<T, dim>(a), b, tolerance);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim, size_t dim0, size_t _00, size_t _01, size_t _02, size_t _03, size_t dim1, size_t _10, size_t _11, size_t _12, size_t _13, typename U>
bool EqualsWithTolerance (Vector_SwizzledSubVector<T, dim0, dim, _00, _01, _02, _03> const& a,
                          Vector_SwizzledSubVector<T, dim1, dim, _10, _11, _12, _13> const& b, U tolerance)
{
    return EqualsWithTolerance(Vector<T, dim>(a), Vector<T, dim>(b), tolerance);
}
//=============================================================================
template<typename T, size_t dim>
T dot(Vector<T, dim> const& a, Vector<T, dim> const& b)
{
    T r(0);
    for(size_t i = 0; i < dim; ++i) { r += a._[i] * b._[i]; }
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
T det(Vector<T, 2> const& a, Vector<T, 2> const& b)
{
    T const r = a._[0] * b._[1] - a._[1] * b._[0];
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
Vector<T, 3> cross(Vector<T, 3> const& a, Vector<T, 3> const& b)
{
    Vector<T, 3> r = Vector<T, 3>(uninitialized);
    r._[0] = a._[1] * b._[2] - a._[2] * b._[1];
    r._[1] = a._[2] * b._[0] - a._[0] * b._[2];
    r._[2] = a._[0] * b._[1] - a._[1] * b._[0];
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
Vector<T, 2> Orthogonal(Vector<T, 2> const& a)
{
    return Vector<T, 2>(-a.y(), a.x());
}
//=============================================================================
#define DEFINE_COMPARE_METHOD(METHOD, OP) \
template<typename T, size_t dim> \
bool METHOD(Vector<T, dim> const& a, Vector<T, dim> const& b) \
{ \
    bool r = true; \
    for(size_t i = 0; i < dim; ++i) { r &= a._[i] OP b._[i]; } \
    return r; \
} \
template<typename T, size_t dim> \
typename Vector<T, dim>::CompResult operator OP(Vector<T, dim> const& a, Vector<T, dim> const& b) \
{ \
    typename Vector<T, dim>::CompResult r; \
    for(size_t i = 0; i < dim; ++i) { r._[i] = a._[i] OP b._[i]; } \
    return r; \
}

DEFINE_COMPARE_METHOD(AllLessStrict, <)
DEFINE_COMPARE_METHOD(AllLessEqual, <=)
DEFINE_COMPARE_METHOD(AllGreaterStrict, >)
DEFINE_COMPARE_METHOD(AllGreaterEqual, >=)
#undef DEFINE_COMPARE_METHOD
//=============================================================================
} // namespace math
//=============================================================================
#if 1
#define DEFINE_FLOAT_TO_FLOAT_METHOD(METHOD) \
    template<typename T, size_t dim> \
    math::Vector<T, dim> METHOD(math::Vector<T, dim> const& v) \
    { \
        math::Vector<T, dim> r(uninitialized); \
        for(size_t i = 0; i < dim; ++i) \
        { \
            r._[i] = METHOD(v._[i]); \
        } \
        return r; \
    }
#else
#define DEFINE_FLOAT_TO_FLOAT_METHOD(METHOD) \
    template<typename T, size_t dim> \
    math::Vector<T, dim> METHOD(math::Vector<T, dim> const& v) \
    { \
        math::Vector<T, dim> r(uninitialized); \
        if(dim > 0) { r._[0] = METHOD(v._[0]); } \
        if(dim > 1) { r._[1] = METHOD(v._[1]); } \
        if(dim > 2) { r._[2] = METHOD(v._[2]); } \
        if(dim > 3) { r._[3] = METHOD(v._[3]); } \
        for(size_t i = 4; i < dim; ++i) \
        { \
            r._[i] = METHOD(v._[i]); \
        } \
        return r; \
    }
#endif
DEFINE_FLOAT_TO_FLOAT_METHOD(floor)
DEFINE_FLOAT_TO_FLOAT_METHOD(round)
DEFINE_FLOAT_TO_FLOAT_METHOD(ceil)
#undef DEFINE_FLOAT_TO_FLOAT_METHOD
//=============================================================================
#define DEFINE_FLOAT_TO_INT_METHOD(METHOD) \
    template<typename T, size_t dim> \
    math::Vector<int, dim> METHOD(math::Vector<T, dim> const& v) \
    { \
        math::Vector<int, dim> r(uninitialized); \
        for(size_t i = 0; i < dim; ++i) \
        { \
            r._[i] = METHOD(v._[i]); \
        } \
        return r; \
    }

DEFINE_FLOAT_TO_INT_METHOD(floori)
DEFINE_FLOAT_TO_INT_METHOD(roundi)
DEFINE_FLOAT_TO_INT_METHOD(ceili)
#undef DEFINE_FLOAT_TO_INT_METHOD
//=============================================================================
namespace componentwise {
using math::Vector;
#define DEFINE_COMPONENTWISE_METHOD(METHOD, PRE_CODE, CODE) \
template<typename T, size_t dim> \
Vector<T, dim> METHOD(Vector<T, dim> const& a, Vector<T, dim> const& b) \
{ \
    PRE_CODE; \
    Vector<T, dim> result = Vector<T, dim>(uninitialized); \
    for(size_t i = 0; i < dim; ++i) \
    { \
        Vector<T, dim>::value_type_for_const_passing l = a._[i]; \
        Vector<T, dim>::value_type_for_const_passing r = b._[i]; \
        result._[i] = CODE; \
    } \
    return result; \
}
// TODO: Vector_SwizzledSubVector
DEFINE_COMPONENTWISE_METHOD(min, using std::min, min(l,r))
DEFINE_COMPONENTWISE_METHOD(max, using std::max, max(l,r))
#undef DEFINE_COMPONENTWISE_METHOD

#define DEFINE_COMPONENTWISE_METHOD(METHOD, PRE_CODE, CODE) \
template<typename T, size_t dim> \
Vector<T, dim> METHOD(Vector<T, dim> const& a) \
{ \
    PRE_CODE; \
    Vector<T, dim> result = Vector<T, dim>(uninitialized); \
    for(size_t i = 0; i < dim; ++i) \
    { \
        Vector<T, dim>::value_type_for_const_passing x = a._[i]; \
        result._[i] = CODE; \
    } \
    return result; \
} \
template<typename T, size_t dim, size_t other_dim, size_t _0, size_t _1, size_t _2, size_t _3> \
Vector<T, dim> METHOD(math::Vector_SwizzledSubVector<T, other_dim, dim, _0, _1, _2, _3> const& a) \
{ \
    return METHOD(Vector<T, dim>(a)); \
}

DEFINE_COMPONENTWISE_METHOD(abs, using std::abs, abs(x))
#undef DEFINE_COMPONENTWISE_METHOD
}
//=============================================================================
typedef math::Vector<float, 2> float2;
typedef math::Vector<float, 3> float3;
typedef math::Vector<float, 4> float4;
//=============================================================================
typedef math::Vector<int, 2> int2;
typedef math::Vector<int, 3> int3;
typedef math::Vector<int, 4> int4;
typedef math::Vector<unsigned int, 2> uint2;
typedef math::Vector<unsigned int, 3> uint3;
typedef math::Vector<unsigned int, 4> uint4;
//=============================================================================
typedef math::Vector<size_t, 2> size_t2;
typedef math::Vector<size_t, 3> size_t3;
typedef math::Vector<size_t, 4> size_t4;
//=============================================================================
typedef math::Vector<i8, 2> sbyte2;
typedef math::Vector<i8, 3> sbyte3;
typedef math::Vector<i8, 4> sbyte4;
typedef math::Vector<u8, 2> ubyte2;
typedef math::Vector<u8, 3> ubyte3;
typedef math::Vector<u8, 4> ubyte4;
//=============================================================================
typedef math::Vector<i16, 2> i16vec2;
typedef math::Vector<i16, 3> i16vec3;
typedef math::Vector<i16, 4> i16vec4;
typedef math::Vector<u16, 2> u16vec2;
typedef math::Vector<u16, 3> u16vec3;
typedef math::Vector<u16, 4> u16vec4;
//=============================================================================
typedef math::Vector<i32, 2> i32vec2;
typedef math::Vector<i32, 3> i32vec3;
typedef math::Vector<i32, 4> i32vec4;
typedef math::Vector<u32, 2> u32vec2;
typedef math::Vector<u32, 3> u32vec3;
typedef math::Vector<u32, 4> u32vec4;
//=============================================================================
typedef math::Vector<bool, 2> bool2;
typedef math::Vector<bool, 3> bool3;
typedef math::Vector<bool, 4> bool4;
//=============================================================================
}

#endif
