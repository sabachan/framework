#ifndef Math_Quaternion_Include_Impl
#error "this file must be used only by Quaternion.h"
#endif

namespace sg {
namespace matrix {
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, math::MatrixOrder order>
math::Matrix<T, dim_m, dim_n, order> InternalRotation(math::TemplateQuaternion<T> const& iQuaternion)
{
    static_assert(std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_integer, "T must be a floating point type");
    static_assert(3 <= dim_m && dim_m <= 4 && dim_m == dim_n, "dimension must be 3x3 or 4x4");
    static_assert(math::MatrixOrder::RowMajor == order, "only row major order is currently supported");

    SG_ASSERT(abs(T(1) - iQuaternion.NormSq()) < std::numeric_limits<T>::epsilon() * T(10));
    math::Matrix<T, dim_m, dim_n, order> m = math::Matrix<T, dim_m, dim_n, order>(uninitialized);
    T const w = iQuaternion.a();
    T const x = iQuaternion.b();
    T const y = iQuaternion.c();
    T const z = iQuaternion.d();
    T const xx = x*x;
    T const yy = y*y;
    T const zz = z*z;
    T const xy = x*y;
    T const xz = x*z;
    T const xw = x*w;
    T const yz = y*z;
    T const yw = y*w;
    T const zw = z*w;

    //cf. http://en.wikipedia.org/wiki/Rotation_matrix#Quaternion
    if(SG_CONSTANT_CONDITION(3 == dim_m))
    {
        T const values[] = {
            1-2*(yy+zz),      2*(xy-zw),      2*(xz+yw),
                2*(xy+zw),    1-2*(xx+zz),      2*(yz-xw),
                2*(xz-yw),      2*(yz+xw),    1-2*(xx+yy)
        };
        /*
        equivalent to
        T const values[] = {
            ww+xx-yy-zz,    2*(xy-zw),      2*(xz+yw),
            2*(xy+zw),      ww-xx+yy-zz,    2*(yz-xw),
            2*(xz-yw),      2*(yz+xw),      ww-xx-yy+zz
        };
        */
        return math::Matrix<T, dim_m, dim_n, order>(values, SG_ARRAYSIZE(values));
    }
    else
    {
        SG_ASSERT(4 == dim_m);
        static const T zero(0);
        static const T one(1);
        T const values[] = {
            1-2*(yy+zz),      2*(xy-zw),      2*(xz+yw),    zero,
                2*(xy+zw),    1-2*(xx+zz),      2*(yz-xw),    zero,
                2*(xz-yw),      2*(yz+xw),    1-2*(xx+yy),    zero,
                    zero,           zero,           zero,     one
        };
        return math::Matrix<T, dim_m, dim_n, order>(values, SG_ARRAYSIZE(values));
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float3x3 Rotation(quaternion const& iQuaternion)
{
    return InternalRotation<float, 3, 3, math::MatrixOrder::RowMajor>(iQuaternion);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float4x4 HomogeneousRotation(quaternion const& iQuaternion)
{
    return InternalRotation<float, 4, 4, math::MatrixOrder::RowMajor>(iQuaternion);
}
//=============================================================================
}
}

