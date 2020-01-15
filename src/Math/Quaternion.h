#ifndef Math_Quaternion_H
#define Math_Quaternion_H

#include "Vector.h"
#include "Matrix.h"
#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/IntTypes.h>
#include <Core/Utils.h>

namespace sg {
namespace math {
//=============================================================================
template<typename T>
class TemplateQuaternion
{
    static_assert(std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_integer, "T must be a floating point type");
public:
    static TemplateQuaternion RepresentRotation(Vector<T, 3> const& iAxis, T iAngleInRadians);
    static TemplateQuaternion RepresentRotation_AssumeNormalisedAxis(Vector<T, 3> const& iAxis, T iAngleInRadians);
    static TemplateQuaternion RepresentRotation(Vector<T, 4> const& iAxis, T iAngleInRadians);
    static TemplateQuaternion RepresentRotation_AssumeNormalisedAxis(Vector<T, 4> const& iAxis, T iAngleInRadians);
public:
    explicit TemplateQuaternion(uninitialized_t) {}
    TemplateQuaternion() { for(size_t i = 0; i < 4; ++i) { m_values[i] = T(); } }
    TemplateQuaternion(T a, T b, T c, T d) { m_values[0] = a; m_values[1] = b; m_values[2] = c; m_values[3] = d; }
    TemplateQuaternion(T const iValues[4]) { for(size_t i = 0; i < 4; ++i) { m_values[i] = iValues[i]; } }

    TemplateQuaternion(TemplateQuaternion const& iOther) { for(size_t i = 0; i < 4; ++i) { m_values[i] = iOther.m_values[i]; } }
    TemplateQuaternion & operator=(TemplateQuaternion const& iOther) { for(size_t i = 0; i < 4; ++i) { m_values[i] = iOther.m_values[i]; } return *this; }

    T const& operator[](size_t i) const { return m_values[i]; }
    T&       operator[](size_t i)       { return m_values[i]; }
    T const& a() const { return m_values[0]; }
    T const& b() const { return m_values[1]; }
    T const& c() const { return m_values[2]; }
    T const& d() const { return m_values[3]; }
    T&       a()       { return m_values[0]; }
    T&       b()       { return m_values[1]; }
    T&       c()       { return m_values[2]; }
    T&       d()       { return m_values[3]; }

    T const& s() const { return m_values[0]; }
    Vector<T, 3> v() const { return Vector<T, 3>(m_values[1], m_values[2], m_values[3]); }

    T NormSq() const { T nsq = T(); for(size_t i = 0; i < 4; ++i) { nsq += m_values[i] * m_values[i]; } return nsq; }
    TemplateQuaternion Normalised();
    TemplateQuaternion Conjugate();
public:
    T m_values[4];
};
//=============================================================================
template<typename T>
TemplateQuaternion<T> TemplateQuaternion<T>::RepresentRotation_AssumeNormalisedAxis(Vector<T, 3> const& iAxis, T iAngleInRadians)
{
    SG_ASSERT(abs(T(1) - iAxis.LengthSq()) < std::numeric_limits<T>::epsilon() * T(10));
    T const c = cos(0.5f * iAngleInRadians);
    T const s = sin(0.5f * iAngleInRadians);
    TemplateQuaternion<T> q = TemplateQuaternion<T>(c, s*iAxis.x(), s*iAxis.y(), s*iAxis.z());
    return q;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
TemplateQuaternion<T> TemplateQuaternion<T>::RepresentRotation(Vector<T, 3> const& iAxis, T iAngleInRadians)
{
    Vector<T, 3> u = iAxis;
    u = u.Normalised();
    return RepresentRotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
TemplateQuaternion<T> TemplateQuaternion<T>::RepresentRotation(Vector<T, 4> const& iAxis, T iAngleInRadians)
{
    SG_ASSERT(0.f == iAxis.w());
    Vector<T, 3> u = iAxis.xyz();
    u = u.Normalised();
    return RepresentRotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
TemplateQuaternion<T> TemplateQuaternion<T>::RepresentRotation_AssumeNormalisedAxis(Vector<T, 4> const& iAxis, T iAngleInRadians)
{
    SG_ASSERT(0.f == iAxis.w());
    float3 u = iAxis.xyz();
    return RepresentRotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//=============================================================================
template<typename T>
TemplateQuaternion<T> TemplateQuaternion<T>::Normalised()
{
    T const normsq = NormSq();
    T const oonorm = 1.f / sqrt(normsq); // TODO: rsqrt
    TemplateQuaternion<T> r = TemplateQuaternion<T>(uninitialized);
    for(size_t i = 0; i < 4; ++i)
    {
        r.m_values[i] = m_values[i] * oonorm;
    }
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
TemplateQuaternion<T> TemplateQuaternion<T>::Conjugate()
{
    TemplateQuaternion<T> r = TemplateQuaternion<T>(uninitialized);
    r.m_values[0] = m_values[0];
    for(size_t i = 1; i < 4; ++i)
    {
        r.m_values[i] = -m_values[i];
    }
    return r;
}
//=============================================================================
template<typename T>
bool operator== (TemplateQuaternion<T> const& a, TemplateQuaternion<T> const& b)
{
    for(size_t i = 0; i < dim; ++i)
    {
        if (a[i] != b[i]) return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T>
bool operator!= (TemplateQuaternion<T> const& a, TemplateQuaternion<T> const& b)
{
    return !(a == b);
}
//=============================================================================
template<typename T>
TemplateQuaternion<T> operator*(TemplateQuaternion<T> const& a, TemplateQuaternion<T> const& b)
{
    TemplateQuaternion<T> r = TemplateQuaternion<T>(uninitialized);
    r.a() = a.a() * b.a() - a.b() * b.b() - a.c() * b.c() - a.d() * b.d();
    r.b() = a.a() * b.b() + a.b() * b.a() + a.c() * b.d() - a.d() * b.c();
    r.c() = a.a() * b.c() - a.b() * b.d() + a.c() * b.a() + a.d() * b.b();
    r.d() = a.a() * b.d() + a.b() * b.c() - a.c() * b.b() + a.d() * b.a();
    return r;
}
//=============================================================================
} // namespace math
//=============================================================================
typedef math::TemplateQuaternion<float> quaternion;
//=============================================================================
namespace matrix {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float3x3 Rotation(quaternion const& iQuaternion);
float4x4 HomogeneousRotation(quaternion const& iQuaternion);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
}

//=============================================================================
#define Math_Quaternion_Include_Impl
#include "Quaternion_Impl.h"
#undef Math_Quaternion_Include_Impl
//=============================================================================

#endif
