#ifndef Math_Box_H
#define Math_Box_H

#include "NumericalUtils.h"
#include "Vector.h"
#include <Core/ArrayView.h>
#include <Core/BitSet.h>
#include <Core/Config.h>
#include <Core/IntTypes.h>
#include <Core/TemplateUtils.h>
#include <Core/Utils.h>
#include <limits>
#include <type_traits>

//=============================================================================
// Definitions
// Box:         Here, it is an axis-aligned box, the cartesian product of
//              intervals.
// Convex:      A convex box is such that min <= max along each axis.
//              A concave box is such that max <= min along each axis. It can
//              be seen as the complementary of a box.
//              A non-convex and non-concave box can be used to represent the
//              cartesian product of intervals and complementary of intervals.
// n-volume:    (NVolume) this is the generalisation of the volume for any
//              dimension, n. For 2D, this is the box area. For 3D, this is the
//              box volume.
//=============================================================================


namespace sg {
//=============================================================================
namespace math {
namespace box_internal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
struct TypeWrapper : public Vector<T, 1>
{
public:
    typedef Vector<T, 1> vector_type;
    typedef typename ConstPassing<vector_type>::type vector_type_for_const_passing;
    typedef T value_type;
    typedef typename vector_type::value_type_for_const_passing value_type_for_const_passing;
public:
    explicit TypeWrapper(uninitialized_t) : Vector(uninitialized) {}
    TypeWrapper() : Vector() {}
    TypeWrapper(value_type_for_const_passing iVal) : Vector(iVal) {}
    TypeWrapper(vector_type_for_const_passing iOther) : Vector(iOther) {}
    TypeWrapper const& operator=(vector_type_for_const_passing iOther) { vector_type::operator=(iOther); return *this; }
    operator value_type() const { return _[0]; }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}
//=============================================================================
namespace math {
//=============================================================================
template<typename T, size_t dim>
class Box
{
    static_assert(1 <= dim, "Box must be at least 1 dimensionnal.");
    typedef typename std::conditional< (2 > dim), box_internal::TypeWrapper<T>, Vector<T, dim> >::type value_type;
    typedef typename std::conditional<(sizeof(value_type)>4), value_type const&, value_type>::type value_type_for_const_passing;
public:
    explicit Box(uninitialized_t) : min(uninitialized), max(uninitialized) {}
    Box() : min(value_type(std::numeric_limits<T>::max())), max(value_type(std::numeric_limits<T>::lowest())) {}
    static Box FromMinMax(value_type_for_const_passing iMin, value_type_for_const_passing iMax) { Box box(uninitialized); box.min = iMin; box.max = iMax; return box; }
    static Box FromMinDelta(value_type_for_const_passing iMin, value_type_for_const_passing iDelta) { Box box(uninitialized); box.min = iMin; box.max = iMin+iDelta; return box; }
    static Box FromMaxDelta(value_type_for_const_passing iMax, value_type_for_const_passing iDelta) { Box box(uninitialized); box.min = iMax-iDelta; box.max = iMax; return box; }
    static Box FromCenterDelta(value_type_for_const_passing iCenter, value_type_for_const_passing iDelta) { Box box(uninitialized); box.min = iCenter-iDelta/2; box.max = box.min + iDelta; return box; }
    template <typename... Ts> static Box BoundingBoxOf(Ts... iArgs) { Box box; box.Grow(std::forward<Ts>(iArgs)...); return box; }
    Box(Box const& iOther) : min(iOther.min), max(iOther.max) {}
    Box & operator=(Box const& iOther) { min = iOther.min; max = iOther.max; return *this; }
    template<typename T2>
    explicit Box(Box<T2, dim> const& iOther) { min = value_type(iOther.min); max = value_type(iOther.max); }

    value_type_for_const_passing Min() const { return min; }
    value_type_for_const_passing Max() const { return max; }
    value_type Delta() const { return max-min; }
    value_type Center() const { return (min+max)/2; }
    value_type Corner(BitSet<dim> const& iCornerDesc) const;
    bool IsConvex() const { return AllGreaterEqual(max, min); }
    T NVolume_AssumeConvex() const;
    T NVolume_NegativeIfNonConvex() const;

    void Grow(value_type_for_const_passing iPoint) { min = componentwise::min(min, iPoint); max = componentwise::max(max, iPoint); }
    void Grow(ArrayView<value_type const> iPoints) { for(value_type_for_const_passing p : iPoints) Grow(p); }
    void Grow(Box const& iBox) { Grow(iBox.min); Grow(iBox.max); }
#define DEFINE_VARIADIC_GROW(TYPE) template <typename... Ts> void Grow(TYPE a, Ts&&... b) { Grow(a); Grow(std::forward<Ts>(b)...); }
    DEFINE_VARIADIC_GROW(value_type_for_const_passing)
    DEFINE_VARIADIC_GROW(ArrayView<value_type const>)
    DEFINE_VARIADIC_GROW(Box const&)
#undef DEFINE_VARIADIC_GROW

    bool Contains(value_type_for_const_passing iPoint) const { return AllGreaterEqual(iPoint, min) && AllLessEqual(iPoint, max); }
    bool Contains(Box const& iBox) const { return AllGreaterEqual(iBox.Min(), min) && AllLessEqual(iBox.Max(), max); }
    bool ContainsStrict(value_type_for_const_passing iPoint) const { return AllGreaterStrict(iPoint, min) && AllLessStrict(iPoint, max); }
    bool ContainsStrict(Box const& iBox) const { return AllGreaterStrict(iBox.Min(), min) && AllLessStrict(iBox.Max(), max); }
    value_type Clamp(value_type_for_const_passing iPoint) const { SG_ASSERT(AllGreaterEqual(max, min)); return componentwise::min(max, componentwise::max(min, iPoint)); }
    value_type VectorToPoint(value_type_for_const_passing iPoint) const { return iPoint - Clamp(iPoint); }

    template<size_t dim2>
    Box<T, dim2> SubBox(size_t i) const;
    template<size_t dim2>
    void SetSubBox(size_t i, Box<T, dim2> const& iSubBox);

#define DEFINE_COMPONENTWISE_OP(OP) \
    Box const& operator OP ## = (Box const& a) \
    { \
        min OP ## = a.min; \
        max OP ## = a.max; \
        return *this; \
    } \
    /* Note that a is taken by copy */ \
    /*  (cf. http://stackoverflow.com/questions/4421706/operator-overloading/4421729#4421729) */ \
    friend Box operator OP (Box a, Box const& b) \
    { \
        return a OP ## = b; \
    } \

    DEFINE_COMPONENTWISE_OP(+)
    DEFINE_COMPONENTWISE_OP(-)
#undef DEFINE_COMPONENTWISE_OP

#define DEFINE_COMPONENTWISE_BOX_VALUE_OP(OP) \
    Box const& operator OP ## = (value_type const& s) \
    { \
        min OP ## = s; \
        max OP ## = s; \
        return *this; \
    } \
    friend Box operator OP (Box a, value_type const& s) \
    { \
        return a OP ## = s; \
    } \
    friend Box operator OP (value_type const& s, Box a) \
    { \
        return a OP ## = s; \
    } \
    Box const& operator OP ## = (T const& s) \
    { \
        min OP ## = s; \
        max OP ## = s; \
        return *this; \
    } \
    friend Box operator OP (Box a, T const& s) \
    { \
        return a OP ## = s; \
    } \
    friend Box operator OP (T const& s, Box a) \
    { \
        return a OP ## = s; \
    }
    DEFINE_COMPONENTWISE_BOX_VALUE_OP(+)
    DEFINE_COMPONENTWISE_BOX_VALUE_OP(-)
    DEFINE_COMPONENTWISE_BOX_VALUE_OP(*)
    DEFINE_COMPONENTWISE_BOX_VALUE_OP(/)
#undef DEFINE_COMPONENTWISE_BOX_VALUE_OP
public:
    // Why (min, max) instead of (min, delta) ?
    // In floats, (min, delta) enables box translation without any risk of
    // changing box size. This property is not enforced when using (min, max).
    // However, (min, max) enable representation of empty box as (+INF, -INF),
    // with no test overhead for growing.
    value_type min;
    value_type max;
};
//=============================================================================
template<typename T, size_t dim>
typename Box<T, dim>::value_type Box<T, dim>::Corner(BitSet<dim> const& iCornerDesc) const
{
    value_type r;
    value_type const* p = &min;
    size_t i = 0;
    for(; i+4 <= dim; i+=4)
    {
        r[i+0] = p[iCornerDesc[i+0]]._[i+0];
        r[i+1] = p[iCornerDesc[i+1]]._[i+1];
        r[i+2] = p[iCornerDesc[i+2]]._[i+2];
        r[i+3] = p[iCornerDesc[i+3]]._[i+3];
    }
    if(i+2 <= dim)
    {
        r[i+0] = p[iCornerDesc[i+0]]._[i+0];
        r[i+1] = p[iCornerDesc[i+1]]._[i+1];
        i += 2;
    }
    if(i < dim)
    {
        r[i+0] = p[iCornerDesc[i+0]]._[i+0];
    }
    return r;
}
//=============================================================================
template<typename T, size_t dim>
T Box<T, dim>::NVolume_AssumeConvex() const
{
    T vol = T(1);
    value_type const delta = Delta();
    for(size_t i = 0; i < dim; ++i)
    {
        SG_ASSERT(T(0) <= delta._[i]);
        vol *= delta._[i];
    }
    return vol;
}
//=============================================================================
template<typename T, size_t dim>
T Box<T, dim>::NVolume_NegativeIfNonConvex() const
{
    T vol = T(1);
    T sign = T(1);
    value_type const delta = Delta();
    for(size_t i = 0; i < dim; ++i)
    {
        if(delta._[i] < T(0))
            sign = T(-1);
        using std::abs;
        vol *= abs(delta._[i]);
    }
    return sign * vol;
}
//=============================================================================
template<typename T, size_t dim>
template<size_t dim2>
Box<T, dim2> Box<T, dim>::SubBox(size_t i) const
{
    Box<T, dim2> r((uninitialized));
    r.min = min.SubVector<dim2>(i);
    r.max = max.SubVector<dim2>(i);
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
template<size_t dim2>
void Box<T, dim>::SetSubBox(size_t i, Box<T, dim2> const& iSubBox)
{
    min.SetSubVector(i, iSubBox.min);
    max.SetSubVector(i, iSubBox.max);
}
//=============================================================================
template<typename T, size_t dim>
bool operator== (Box<T, dim> const& a, Box<T, dim> const& b)
{
    return a.min == b.min && a.max == b.max;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
bool operator!= (Box<T, dim> const& a, Box<T, dim> const& b)
{
    return !(a == b);
}
//=============================================================================
template<typename T, size_t dim>
Box<T, dim> CartesianProduct (Box<T, dim> const& a) { return a; }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim, size_t... dims>
Box<T, Sum<size_t, dim, dims...>::value> CartesianProduct (Box<T, dim> const& a, Box<T, dims> const&... bs)
{
    Box<T, Sum<size_t, dims...>::value> c = CartesianProduct(bs...);
    Box<T, Sum<size_t, dim, dims...>::value> r(uninitialized);
    r.min.SetSubVector(0, a.min);
    r.min.SetSubVector(dim, c.min);
    r.max.SetSubVector(0, a.max);
    r.max.SetSubVector(dim, c.max);
    return r;
}
//=============================================================================
template<typename T, size_t dim>
Box<T, dim> Intersection(Box<T, dim> const& a, Box<T, dim> const& b)
{
    Box<T, dim> r;
    r.min = componentwise::max(a.min, b.min);
    r.max = componentwise::min(a.max, b.max);
    return r;
}
//=============================================================================
template<typename T, size_t dim>
bool IntersectOrTouch(Box<T, dim> const& a, Box<T, dim> const& b)
{
    Box<T, dim> r;
    if (a.min.x() > b.max.x())
        return false;
    if (a.min.y() > b.max.y())
        return false;
    if (a.max.x() < b.min.x())
        return false;
    if (a.max.y() < b.min.y())
        return false;
    return true;
}
//=============================================================================
template<typename T, size_t dim>
bool IntersectStrict(Box<T, dim> const& a, Box<T, dim> const& b)
{
    Box<T, dim> r;
    if (a.min.x() >= b.max.x())
        return false;
    if (a.min.y() >= b.max.y())
        return false;
    if (a.max.x() <= b.min.x())
        return false;
    if (a.max.y() <= b.min.y())
        return false;
    return true;
}
//=============================================================================
} // namespace math
//=============================================================================
typedef math::Box<float, 1> box1f;
typedef math::Box<float, 2> box2f;
typedef math::Box<float, 3> box3f;
typedef math::Box<float, 4> box4f;
typedef math::Box<int, 1> box1i;
typedef math::Box<int, 2> box2i;
typedef math::Box<int, 3> box3i;
typedef math::Box<int, 4> box4i;
typedef math::Box<unsigned int, 1> box1u;
typedef math::Box<unsigned int, 2> box2u;
typedef math::Box<unsigned int, 3> box3u;
typedef math::Box<unsigned int, 4> box4u;
//=============================================================================
typedef math::Box<float, 1> floatbox1;
typedef math::Box<float, 2> floatbox2;
typedef math::Box<float, 3> floatbox3;
typedef math::Box<float, 4> floatbox4;
typedef math::Box<int, 1> intbox1;
typedef math::Box<int, 2> intbox2;
typedef math::Box<int, 3> intbox3;
typedef math::Box<int, 4> intbox4;
typedef math::Box<unsigned int, 1> uintbox1;
typedef math::Box<unsigned int, 2> uintbox2;
typedef math::Box<unsigned int, 3> uintbox3;
typedef math::Box<unsigned int, 4> uintbox4;
typedef math::Box<size_t, 1> size_tbox1;
typedef math::Box<size_t, 2> size_tbox2;
typedef math::Box<size_t, 3> size_tbox3;
typedef math::Box<size_t, 4> size_tbox4;
typedef math::Box<i32, 1> i32box1;
typedef math::Box<i32, 2> i32box2;
typedef math::Box<i32, 3> i32box3;
typedef math::Box<i32, 4> i32box4;
typedef math::Box<u32, 1> u32box1;
typedef math::Box<u32, 2> u32box2;
typedef math::Box<u32, 3> u32box3;
typedef math::Box<u32, 4> u32box4;
//=============================================================================
}

#endif
