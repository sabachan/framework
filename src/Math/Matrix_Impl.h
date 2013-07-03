#ifndef Math_Matrix_Include_Impl
#error "this file must be used only by Matrix.h"
#endif

namespace sg {
namespace matrix {
//=============================================================================
inline float2x2 Rotation(float iAngleInRadians)
{
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
         c, -s,
         s,  c,
    };
    return float2x2(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float3x3 RotationX(float iAngleInRadians)
{
    // IMPORTANT NOTE: here, the convention is that we describe direct-oriented
    // rotations in direct-oriented frames (that some people say to be right-handed),
    // ie, rotation is counter-clockwise for positive angle, when looking from
    // axis direction.
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
        1.f, 0.f, 0.f,
        0.f,   c,  -s,
        0.f,   s,   c,
    };
    return float3x3(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float3x3 RotationY(float iAngleInRadians)
{
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
          c, 0.f,   s,
        0.f, 1.f, 0.f,
         -s, 0.f,   c,
    };
    return float3x3(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float3x3 RotationZ(float iAngleInRadians)
{
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
          c,  -s, 0.f,
          s,   c, 0.f,
        0.f, 0.f, 1.f,
    };
    return float3x3(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotationX(float iAngleInRadians)
{
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
        1.f, 0.f, 0.f, 0.f,
        0.f,   c,  -s, 0.f,
        0.f,   s,   c, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
    return float4x4(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotationY(float iAngleInRadians)
{
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
          c, 0.f,   s, 0.f,
        0.f, 1.f, 0.f, 0.f,
         -s, 0.f,   c, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
    return float4x4(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotationZ(float iAngleInRadians)
{
    float const c = cos(iAngleInRadians);
    float const s = sin(iAngleInRadians);
    float const values[] = {
          c,  -s, 0.f, 0.f,
          s,   c, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
    return float4x4(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, math::MatrixOrder order>
math::Matrix<T, dim_m, dim_n, order> InternalRotation_AssumeNormalisedAxis(math::Vector<T, 3> const& iAxis, T iAngleInRadians)
{
    static_assert(std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_integer, "T must be a floating point type");
    static_assert(3 <= dim_m && dim_m <= 4 && dim_m == dim_n, "dimension must be 3x3 or 4x4");
    static_assert(math::MatrixOrder::RowMajor == order, "only row major order is currently supported");

    SG_ASSERT(abs(T(1) - iAxis.LengthSq()) < std::numeric_limits<T>::epsilon() * T(10));
    T const c = cos(iAngleInRadians);
    T const s = sin(iAngleInRadians);
    T const omc = (T(1)-c);
    T const ux = iAxis.x();
    T const uy = iAxis.y();
    T const uz = iAxis.z();
    T const uxx = iAxis.x()*iAxis.x();
    T const uyy = iAxis.y()*iAxis.y();
    T const uzz = iAxis.z()*iAxis.z();
    T const uxy = iAxis.x()*iAxis.y();
    T const uxz = iAxis.x()*iAxis.z();
    T const uyz = iAxis.y()*iAxis.z();
    // cf. http://en.wikipedia.org/wiki/Rotation_matrix
    if(SG_CONSTANT_CONDITION(3 == dim_m))
    {
        T const values[] = {
            c + uxx*omc,        uxy*omc - uz*s,     uxz*omc + uy*s,
            uxy*omc + uz*s,     c + uyy*omc,        uyz*omc - ux*s,
            uxz*omc - uy*s,     uyz*omc + ux*s,     c + uzz*omc
        };
        return math::Matrix<T, dim_m, dim_n, order>(values, SG_ARRAYSIZE(values));
    }
    else
    {
        SG_ASSERT(4 == dim_m);
        static const T zero(0);
        static const T one(1);
        T const values[] = {
            c + uxx*omc,        uxy*omc - uz*s,     uxz*omc + uy*s,     zero,
            uxy*omc + uz*s,     c + uyy*omc,        uyz*omc - ux*s,     zero,
            uxz*omc - uy*s,     uyz*omc + ux*s,     c + uzz*omc,        zero,
            zero,               zero,               zero,               one
        };
        return math::Matrix<T, dim_m, dim_n, order>(values, SG_ARRAYSIZE(values));
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float3x3 Rotation(float3 const& iAxis, float iAngleInRadians)
{
    float3 u = iAxis;
    u = u.Normalised();
    return Rotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float3x3 Rotation_AssumeNormalisedAxis(float3 const& iAxis, float iAngleInRadians)
{
    return InternalRotation_AssumeNormalisedAxis<float, 3, 3, math::MatrixOrder::RowMajor>(iAxis, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotation(float3 const& iAxis, float iAngleInRadians)
{
    float3 u = iAxis;
    u = u.Normalised();
    return HomogeneousRotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotation_AssumeNormalisedAxis(float3 const& iAxis, float iAngleInRadians)
{
    return InternalRotation_AssumeNormalisedAxis<float, 4, 4, math::MatrixOrder::RowMajor>(iAxis, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotation(float4 const& iAxis, float iAngleInRadians)
{
    SG_ASSERT(0.f == iAxis.w());
    float3 u = iAxis.xyz();
    u = u.Normalised();
    return HomogeneousRotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotation_AssumeNormalisedAxis(float4 const& iAxis, float iAngleInRadians)
{
    SG_ASSERT(0.f == iAxis.w());
    float3 u = iAxis.xyz();
    return HomogeneousRotation_AssumeNormalisedAxis(u, iAngleInRadians);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousTranslation(float3 const& iTranslation)
{
    float const values[] = {
        1.f, 0.f, 0.f, iTranslation.x(),
        0.f, 1.f, 0.f, iTranslation.y(),
        0.f, 0.f, 1.f, iTranslation.z(),
        0.f, 0.f, 0.f, 1.f,
    };
    return float4x4(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousTranslation(float4 const& iTranslation)
{
    SG_ASSERT(0.f == iTranslation.w());
    return HomogeneousTranslation(iTranslation.xyz());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousScale(float iScale)
{
    float const s = iScale;
    float const values[] = {
          s, 0.f, 0.f, 0.f,
        0.f,   s, 0.f, 0.f,
        0.f, 0.f,   s, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
    return float4x4(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousScale(float3 const& iScale)
{
    float const sx = iScale.x();
    float const sy = iScale.y();
    float const sz = iScale.z();
    float const values[] = {
         sx, 0.f, 0.f, 0.f,
        0.f,  sy, 0.f, 0.f,
        0.f, 0.f,  sz, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
    return float4x4(values);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim>
math::Matrix<T, dim, dim, math::MatrixOrder::RowMajor> Diagonal(math::Vector<T, dim> const& iDiagonal)
{
    typedef math::Matrix<T, dim, dim, math::MatrixOrder::RowMajor> matrix_type;
    matrix_type r(uninitialized);
    T* v = r.Data();
    *v = iDiagonal[0];
    ++v;
    for(size_t i = 1; i < dim; ++i)
    {
        for(size_t j = 0; j < dim; ++j)
        {
            *v = 0;
            ++v;
        }
        *v = iDiagonal[i];
        ++v;
    }
    SG_ASSERT(v - r.Data() == matrix_type::value_count);
    return r;
}
//=============================================================================
}
}
