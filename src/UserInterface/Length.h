#ifndef UserInterface_Length_H
#define UserInterface_Length_H

#include <Math/Box.h>
#include <Math/Vector.h>
#include <Reflection/BaseClass.h>

namespace sg {
namespace ui {
//=============================================================================
namespace lengthinternal {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, bool U, bool M, bool R>
struct Temp
{
    static_assert(std::is_same<T, float>::value
                  || std::is_same<T, float2>::value
                  || std::is_same<T, box2f>::value
                  , "Unsupported type for ui::length");
public:
    static bool const use_unit = U;
    static bool const use_magnifiable = M;
    static bool const use_relative = R;
    static size_t const DATA_SIZE = (U?1:0) + (M?1:0) + (R?1:0);
    T data[DATA_SIZE];

public:
    float Resolve(float iMagnification, float iParentSize) const
    {
        T r = T();
        size_t i = 0;
        if(use_unit) r += data[i++];
        if(use_magnifiable) r += data[i++] * iMagnification;
        if(use_relative) r += data[i++] * iParentSize;
        return r;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, bool U, bool M, bool R, bool U2, bool M2, bool R2>
inline Temp<T, U||U2, M||M2, R||R2> operator + (Temp<T,U,M,R> const& a, Temp<T, U2, M2, R2> const& b)
{
    Temp<T, U||U2, M||M2, R||R2> r;
    size_t i1 = 0, i2 = 0, ir = 0;
    if(U && U2) r.data[ir++] = a.data[i1++] + b.data[i2++];
    else if(U)  r.data[ir++] = a.data[i1++];
    else if(U2) r.data[ir++] = b.data[i2++];
    if(M && M2) r.data[ir++] = a.data[i1++] + b.data[i2++];
    else if(M)  r.data[ir++] = a.data[i1++];
    else if(M2) r.data[ir++] = b.data[i2++];
    if(R && R2) r.data[ir++] = a.data[i1++] + b.data[i2++];
    else if(R)  r.data[ir++] = a.data[i1++];
    else if(R2) r.data[ir++] = b.data[i2++];
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
#define SG_UI_DEFINE_LENGTH_HELPERS_FOR_TYPE(TYPE) \
inline lengthinternal::Temp<TYPE,1,0,0> Unit       (TYPE const& x) { lengthinternal::Temp<TYPE,1,0,0> r; r.data[0] = x; return r; } \
inline lengthinternal::Temp<TYPE,0,1,0> Magnifiable(TYPE const& x) { lengthinternal::Temp<TYPE,0,1,0> r; r.data[0] = x; return r; } \
inline lengthinternal::Temp<TYPE,0,0,1> Relative   (TYPE const& x) { lengthinternal::Temp<TYPE,0,0,1> r; r.data[0] = x; return r; }

SG_UI_DEFINE_LENGTH_HELPERS_FOR_TYPE(float)
SG_UI_DEFINE_LENGTH_HELPERS_FOR_TYPE(float2)
SG_UI_DEFINE_LENGTH_HELPERS_FOR_TYPE(box2f)
#undef SG_UI_DEFINE_LENGTH_HELPERS_FOR_TYPE
//=============================================================================
struct Length : public reflection::BaseType
{
    REFLECTION_TYPE_HEADER(Length, reflection::BaseType)
public:
    float unit;
    float magnifiable;
    float relative;

public:
    Length() : unit(0), magnifiable(0), relative(0) {}
    Length(float u, float m, float r) : unit(u), magnifiable(m), relative(r) {}
    float Resolve(float iMagnification, float iParentSize) const
    {
        return unit + magnifiable * iMagnification + relative * iParentSize;
    }

    template <bool U, bool M, bool R>
    Length(lengthinternal::Temp<float,U,M,R> const& iTmp)
    {
        size_t i = 0;
        this->unit = U ? iTmp.data[i++] : 0;
        this->magnifiable = M ? iTmp.data[i++] : 0;
        this->relative = R ? iTmp.data[i++] : 0;
    }

    Length operator - () const
    {
        Length r;
        r.unit = -unit;
        r.magnifiable = -magnifiable;
        r.relative = -relative;
        return r;
    }

#define DEFINE_OP(OP) \
    Length const& operator OP ## = (Length const& a) \
    { \
        this->unit += a.unit; \
        this->magnifiable += a.magnifiable; \
        this->relative += a.relative; \
        return *this; \
    } \
    /* Note that a is taken by copy */ \
    /*  (cf. http://stackoverflow.com/questions/4421706/operator-overloading/4421729#4421729) */ \
    friend Length operator OP (Length a, Length const& b) \
    { \
        return a OP ## = b; \
    }

    DEFINE_OP(+)
    DEFINE_OP(-)
#undef DEFINE_OP
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline bool operator == (Length const& a, Length const& b) { return a.unit == b.unit && a.magnifiable == b.magnifiable && a.relative == b.relative; }
inline bool operator != (Length const& a, Length const& b) { return a.unit != b.unit || a.magnifiable != b.magnifiable || a.relative != b.relative; }
//=============================================================================
class Length2 : public math::Vector<Length, 2>
              , public reflection::BaseType
{
    REFLECTION_TYPE_HEADER(Length2, reflection::BaseType)
    typedef math::Vector<Length, 2> parent_type;
public:
    Length2() : parent_type() {}
    Length2(parent_type const& v) : parent_type(v) {}
    explicit Length2(Length const& broadcasted) : parent_type(broadcasted) {}
    Length2(Length const& x, Length const& y) : parent_type(x, y) {}
    float2 Resolve(float iMagnification, float2 const& iParentSize) const
    {
        return float2(_[0].Resolve(iMagnification, iParentSize._[0]), _[1].Resolve(iMagnification, iParentSize._[1]));
    }

    template <bool U, bool M, bool R>
    Length2(lengthinternal::Temp<float2,U,M,R> const& iTmp)
    {
        size_t i = 0;
        this->x().unit        = U ? iTmp.data[i  ].x() : 0;
        this->y().unit        = U ? iTmp.data[i++].y() : 0;
        this->x().magnifiable = M ? iTmp.data[i  ].x() : 0;
        this->y().magnifiable = M ? iTmp.data[i++].y() : 0;
        this->x().relative    = R ? iTmp.data[i  ].x() : 0;
        this->y().relative    = R ? iTmp.data[i++].y() : 0;
    }

#define DEFINE_COMPONENTWISE_OP(OP) \
    Length2 const& operator OP ## = (Length2 const& a) \
    { \
        static_cast<parent_type&>(*this) OP ## = static_cast<parent_type const&>(a); \
        return *this; \
    } \
    /* Note that a is taken by copy */ \
    /*  (cf. http://stackoverflow.com/questions/4421706/operator-overloading/4421729#4421729) */ \
    friend Length2 operator OP (Length2 a, Length2 const& b) \
    { \
        return a OP ## = b; \
    }

    // NB: it is possible to write vetor + scalar, and scalar will be brodcasted.
    // This is the same behavior as in DX shaders.
    DEFINE_COMPONENTWISE_OP(+)
    DEFINE_COMPONENTWISE_OP(-)
#undef DEFINE_COMPONENTWISE_OP
};
//=============================================================================
class LengthBox2 : public math::Box<Length, 2>
                 , public reflection::BaseType
{
    REFLECTION_TYPE_HEADER(LengthBox2, reflection::BaseType)
    typedef math::Box<Length, 2> parent_type;
public:
    explicit LengthBox2(uninitialized_t) : parent_type(uninitialized) {}
    LengthBox2() : parent_type(parent_type::FromMinMax(Length2(), Length2())) {}
    LengthBox2(parent_type const& v) : parent_type(v) {}
    static LengthBox2 FromMinMax(Length2 const& iMin, Length2 const& iMax) { LengthBox2 box(uninitialized); box.min = iMin; box.max = iMax; return box; }
    static LengthBox2 FromMinDelta(Length2 const& iMin, Length2 const& iDelta) { LengthBox2 box(uninitialized); box.min = iMin; box.max = iMin+iDelta; return box; }
    static LengthBox2 FromMaxDelta(Length2 const& iMax, Length2 const& iDelta) { LengthBox2 box(uninitialized); box.min = iMax-iDelta; box.max = iMax; return box; }
    //TODO: static LengthBox2 FromCenterDelta(Length2 const& iCenter, Length2 const& iDelta) { LengthBox2 box(uninitialized); box.min = iCenter-iDelta/2; box.max = box.min + iDelta; return box; }
    box2f Resolve(float iMagnification, float2 const& iParentSize) const
    {
        return box2f::FromMinMax(
            float2(Min()._[0].Resolve(iMagnification, iParentSize._[0]), Min()._[1].Resolve(iMagnification, iParentSize._[1])),
            float2(Max()._[0].Resolve(iMagnification, iParentSize._[0]), Max()._[1].Resolve(iMagnification, iParentSize._[1])));
    }

    template <bool U, bool M, bool R>
    LengthBox2(lengthinternal::Temp<box2f,U,M,R> const& iTmp)
    {
        size_t i = 0;
        this->min._[0].unit   = U ? iTmp.data[i  ].min._[0] : 0;
        this->min._[1].unit   = U ? iTmp.data[i  ].min._[1] : 0;
        this->max._[0].unit   = U ? iTmp.data[i  ].max._[0] : 0;
        this->max._[1].unit   = U ? iTmp.data[i++].max._[1] : 0;
        this->min._[0].magnifiable   = U ? iTmp.data[i  ].min._[0] : 0;
        this->min._[1].magnifiable   = U ? iTmp.data[i  ].min._[1] : 0;
        this->max._[0].magnifiable   = U ? iTmp.data[i  ].max._[0] : 0;
        this->max._[1].magnifiable   = U ? iTmp.data[i++].max._[1] : 0;
        this->min._[0].relative   = U ? iTmp.data[i  ].min._[0] : 0;
        this->min._[1].relative   = U ? iTmp.data[i  ].min._[1] : 0;
        this->max._[0].relative   = U ? iTmp.data[i  ].max._[0] : 0;
        this->max._[1].relative   = U ? iTmp.data[i++].max._[1] : 0;
    }
};
//=============================================================================
}
}

#endif
