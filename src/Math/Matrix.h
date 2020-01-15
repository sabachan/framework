#ifndef Math_Matrix_H
#define Math_Matrix_H

#include "NumericalUtils.h"
#include "Vector.h"
#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/IntTypes.h>
#include <Core/TemplateUtils.h>
#include <Core/Utils.h>
#include <type_traits>

namespace sg {
namespace math {
//=============================================================================
enum class MatrixOrder { RowMajor, ColumnMajor };
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order = MatrixOrder::RowMajor>
class Matrix
{
    template<MatrixOrder order> struct MatrixTransposedOrder { };
    template<> struct MatrixTransposedOrder<MatrixOrder::RowMajor> { static const MatrixOrder value = MatrixOrder::ColumnMajor; };
    template<> struct MatrixTransposedOrder<MatrixOrder::ColumnMajor> { static const MatrixOrder value = MatrixOrder::RowMajor; };
public:
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
    typedef Vector<T, dim_n> row_type;
    typedef Vector<T, dim_m> col_type;
    static size_t const value_count = dim_m * dim_n;
public:
    static Matrix Identity();
public:
    explicit Matrix(uninitialized_t) {}
    Matrix() { for(size_t i = 0; i < value_count; ++i) { m_values[i] = T(); } }
    Matrix(T const(& iValues)[value_count]) { for(size_t i = 0; i < value_count; ++i) { m_values[i] = iValues[i]; } }
    Matrix(T const* iValues, size_t iNValues) { SG_ASSERT_AND_UNUSED(iNValues == value_count); for(size_t i = 0; i < value_count; ++i) { m_values[i] = iValues[i]; } }

    Matrix(Matrix const& iOther) { for(size_t i = 0; i < value_count; ++i) { m_values[i] = iOther.m_values[i]; } }
    Matrix & operator=(Matrix const& iOther) { for(size_t i = 0; i < value_count; ++i) { m_values[i] = iOther.m_values[i]; } return *this; }

    template<typename U>
    explicit Matrix(Matrix<U, dim_m, dim_n, order> const& iOther) { for(size_t i = 0; i < value_count; ++i) { m_values[i] = T(iOther.Data()[i]); } }

    Matrix(Matrix<T, dim_m, dim_n, MatrixTransposedOrder<order>::value> const& iOther) { for(size_t i = 0; i < dim_m; ++i) for(size_t j = 0; j < dim_n; ++j) { (*this)(i,j) = iOther(i,j); } }
    Matrix & operator=(Matrix<T, dim_m, dim_n, MatrixTransposedOrder<order>::value> const& iOther) { for(size_t i = 0; i < dim_m; ++i) for(size_t j = 0; j < dim_n; ++j) { (*this)(i,j) = iOther(i,j); } return *this; }

    row_type Row(size_t i) const;
    col_type Col(size_t i) const;
    void SetCol(size_t i, col_type const& v);
    void SetRow(size_t i, row_type const& v);

    value_type_for_const_passing operator () (size_t i, size_t j) const;
    T& operator () (size_t i, size_t j);

#define DEFINE_COMPONENTWISE_COMPOUND_ASSIGMENT_OP(OP_EQ) \
    template<MatrixOrder order_a> \
    Matrix const& operator OP_EQ (Matrix<T, dim_m, dim_n, order_a> const& a) \
    { \
        for(size_t i = 0; i < value_count; ++i) { m_values[i] OP_EQ a[i]; } \
        return *this; \
    }
    DEFINE_COMPONENTWISE_COMPOUND_ASSIGMENT_OP(+=)
    DEFINE_COMPONENTWISE_COMPOUND_ASSIGMENT_OP(-=)
#undef DEFINE_COMPONENTWISE_COMPOUND_ASSIGMENT_OP

#define DEFINE_COMPONENTWISE_SCALAR_COMPOUND_ASSIGMENT_OP(OP_EQ) \
    Matrix const& operator OP_EQ (T const& s) \
    { \
        for(size_t i = 0; i < value_count; ++i) { m_values[i] OP_EQ s; } \
        return *this; \
    }
    DEFINE_COMPONENTWISE_SCALAR_COMPOUND_ASSIGMENT_OP(*=)
#undef DEFINE_COMPONENTWISE_SCALAR_COMPOUND_ASSIGMENT_OP

    Matrix<T, dim_n, dim_m, MatrixTransposedOrder<order>::value> Transposed() const { return Matrix<T, dim_n, dim_m, MatrixTransposedOrder<order>::value>(m_values); }

    template<size_t dim_m_2, size_t dim_n_2>
    Matrix<T, dim_m_2, dim_n_2, order> SubMatrix(size_t i, size_t j) const;
    template<size_t dim_m_2, size_t dim_n_2, MatrixOrder order_2>
    void SetSubMatrix(size_t i, size_t j, Matrix<T, dim_m_2, dim_n_2, order_2> const& iSubMatrix);

    T* Data() { return m_values; }
    T const* Data() const { return m_values; }
private:
    T m_values[value_count];
};
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
T& Matrix<T, dim_m, dim_n, order>::operator () (size_t i, size_t j)
{
    SG_ASSERT(i < dim_m);
    SG_ASSERT(j < dim_n);
    switch(order)
    {
    case MatrixOrder::RowMajor:
        return m_values[j + dim_n * i];
    case MatrixOrder::ColumnMajor:
        return m_values[i + dim_m * j];
    default:
        SG_ASSUME_NOT_REACHED();
        SG_CODE_FOR_ASSERT(return m_values[0];)
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
typename Matrix<T, dim_m, dim_n, order>::value_type_for_const_passing Matrix<T, dim_m, dim_n, order>::operator () (size_t i, size_t j) const
{
    return const_cast<Matrix<T, dim_m, dim_n, order>&>(*this)(i,j);
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
typename Matrix<T, dim_m, dim_n, order>::row_type Matrix<T, dim_m, dim_n, order>::Row(size_t i) const
{
    SG_ASSERT(i < dim_m);
    switch(order)
    {
    case MatrixOrder::RowMajor:
        return *reinterpret_cast<row_type const*>(m_values + dim_n * i);
    case MatrixOrder::ColumnMajor:
        {
            row_type row(uninitialized);
            for(size_t c = 0; c < dim_n; ++c)
            {
                row._[c] = m_values[c * dim_m + i];
            }
            return row;
        }
    default:
        SG_ASSUME_NOT_REACHED();
        SG_CODE_FOR_ASSERT(return row_type();)
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
typename Matrix<T, dim_m, dim_n, order>::col_type Matrix<T, dim_m, dim_n, order>::Col(size_t i) const
{
    SG_ASSERT(i < dim_n);
    switch(order)
    {
    case MatrixOrder::RowMajor:
        {
            col_type col(uninitialized);
            for(size_t r = 0; r < dim_m; ++r)
            {
                col._[r] = m_values[r * dim_n + i];
            }
            return col;
        }
    case MatrixOrder::ColumnMajor:
        return *reinterpret_cast<col_type const*>(m_values + dim_m * i);
    default:
        SG_ASSUME_NOT_REACHED();
        SG_CODE_FOR_ASSERT(return col_type();)
    }
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
void Matrix<T, dim_m, dim_n, order>::SetRow(size_t i, row_type const& v)
{
    SG_ASSERT(i < dim_m);
    switch(order)
    {
    case MatrixOrder::RowMajor:
        {
            for(size_t c = 0; c < dim_n; ++c)
            {
                 m_values[c + i * dim_n] = v._[c];
            }
        }
        break;
    case MatrixOrder::ColumnMajor:
        {
            for(size_t c = 0; c < dim_n; ++c)
            {
                 m_values[c * dim_m + i] = v._[c];
            }
        }
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
void Matrix<T, dim_m, dim_n, order>::SetCol(size_t i, col_type const& v)
{
    SG_ASSERT(i < dim_n);
    switch(order)
    {
    case MatrixOrder::RowMajor:
        {
            for(size_t r = 0; r < dim_m; ++r)
            {
                 m_values[r * dim_n + i] = v._[r];
            }
        }
        break;
    case MatrixOrder::ColumnMajor:
        {
            for(size_t r = 0; r < dim_m; ++r)
            {
                 m_values[r + i * dim_m] = v._[r];
            }
        }
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order_a, MatrixOrder order_b>
bool operator == (Matrix<T, dim_m, dim_n, order_a> const a, Matrix<T, dim_m, dim_n, order_b> const& b)
{
    for(size_t i = 0; i < dim_m; ++i)
    {
        for(size_t j = 0; j < dim_n; ++j)
        {
            if (a(i,j) != b(i,j)) return false;
        }
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n>
bool operator == (Matrix<T, dim_m, dim_n, MatrixOrder::ColumnMajor> const a, Matrix<T, dim_m, dim_n, MatrixOrder::ColumnMajor> const& b)
{
    for(size_t j = 0; j < dim_n; ++j)
    {
        for(size_t i = 0; i < dim_m; ++i)
        {
            if (a(i,j) != b(i,j)) return false;
        }
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order_a, MatrixOrder order_b>
bool operator != (Matrix<T, dim_m, dim_n, order_a> const a, Matrix<T, dim_m, dim_n, order_b> const& b)
{
    return !(a == b);
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
Vector<T, dim_m> operator * (Matrix<T, dim_m, dim_n, order> const m, Vector<T, dim_n> const& v)
{
    Vector<T, dim_m> r;
    for(size_t i = 0; i < dim_m; ++i)
    {
        T r_i = T();
        for(size_t j = 0; j < dim_n; ++j)
        {
            r_i += m(i,j) * v[j];
        }
        r[i] = r_i;
    }
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim, MatrixOrder order>
T operator * (Matrix<T, 1, dim, order> const u, Vector<T, dim> const& v)
{
    T r = T();
    for(size_t i = 0; i < dim; ++i)
    {
        r += u(0,i) * v[i];
    }
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
Matrix<T, 1, dim_n, order> operator * (Vector<T, dim_m> const& v, Matrix<T, dim_m, dim_n, order> const m)
{
    Matrix<T, 1, dim_n, order> r;
    for(size_t j = 0; j < dim_n; ++j)
    {
        T r_j = T();
        for(size_t i = 0; i < dim_m; ++i)
        {
            r_j += v[i] * m(i,j);
        }
        r(0,j) = r_j;
    }
    return r;
}
//=============================================================================
template<typename T, size_t dim_0, size_t dim_1, size_t dim_2, MatrixOrder order_a, MatrixOrder order_b>
Matrix<T, dim_0, dim_2, order_a> operator * (Matrix<T, dim_0, dim_1, order_a> const a, Matrix<T, dim_1, dim_2, order_b> const& b)
{
    Matrix<T, dim_0, dim_2, order_a> r;
    for(size_t i = 0; i < dim_0; ++i)
    {
        for(size_t j = 0; j < dim_2; ++j)
        {
            T r_ij = T();
            for(size_t k = 0; k < dim_1; ++k)
            {
                r_ij += a(i,k) * b(k,j);
            }
            r(i,j) = r_ij;
        }
    }
    return r;
}
//=============================================================================
#define DEFINE_COMPONENTWISE_BINARY_OP(OP) \
    template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order_a, MatrixOrder order_b> \
    /* Note that a is taken by copy */ \
    /*  (cf. http://stackoverflow.com/questions/4421706/operator-overloading/4421729#4421729) */ \
    Matrix<T, dim_m, dim_n, order_a> operator OP (Matrix<T, dim_m, dim_n, order_a> const a, Matrix<T, dim_m, dim_n, order_b> const& b) \
    { \
        return a OP= b; \
    }

DEFINE_COMPONENTWISE_BINARY_OP(+)
DEFINE_COMPONENTWISE_BINARY_OP(-)
#undef DEFINE_COMPONENTWISE_BINARY_OP

#define DEFINE_COMPONENTWISE_SCALAR_BINARY_OP(OP) \
    template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order> \
    Matrix<T, dim_m, dim_n, order> operator OP (Matrix<T, dim_m, dim_n, order> a, T const& s) \
    { \
        return a OP= s; \
    } \
    template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order> \
    Matrix<T, dim_m, dim_n, order> operator OP (T const& s, Matrix<T, dim_m, dim_n, order> a) \
    { \
        return a OP= s; \
    }

DEFINE_COMPONENTWISE_SCALAR_BINARY_OP(+)
DEFINE_COMPONENTWISE_SCALAR_BINARY_OP(-)
DEFINE_COMPONENTWISE_SCALAR_BINARY_OP(*)
DEFINE_COMPONENTWISE_SCALAR_BINARY_OP(/)
#undef DEFINE_COMPONENTWISE_SCALAR_BINARY_OP
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
Matrix<T, dim_m, dim_n, order> Matrix<T, dim_m, dim_n, order>::Identity()
{
    static_assert(dim_m == dim_n, "identity must be a square matrix.");
    Matrix<T, dim_m, dim_n, order> identity = Matrix<T, dim_m, dim_n, order>(uninitialized);
    for(size_t i = 0; i < dim_m; ++i)
    {
        for(size_t j = 0; j < dim_n; ++j)
        {
            identity(i,j) = (i==j) ? T(1) : T(0);
        }
    }
    return identity;
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
template<size_t dim_m_2, size_t dim_n_2>
Matrix<T, dim_m_2, dim_n_2, order> Matrix<T, dim_m, dim_n, order>::SubMatrix(size_t i, size_t j) const
{
    static_assert(dim_m_2 <= dim_m, "");
    static_assert(dim_n_2 <= dim_n, "");
    SG_ASSERT(i + dim_m_2 <= dim_m);
    SG_ASSERT(j + dim_n_2 <= dim_n);
    Matrix<T, dim_m_2, dim_n_2, order> subMat;
    for(size_t ki = 0; ki < dim_m_2; ++ki)
    {
        for(size_t kj = 0; kj < dim_n_2; ++kj)
        {
            subMat(ki,kj) = operator()(ki+i, kj+j);
        }
    }
    return subMat;
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
template<size_t dim_m_2, size_t dim_n_2, MatrixOrder order_2>
void Matrix<T, dim_m, dim_n, order>::SetSubMatrix(size_t i, size_t j, Matrix<T, dim_m_2, dim_n_2, order_2> const& iSubMatrix)
{
    static_assert(dim_m_2 <= dim_m, "");
    static_assert(dim_n_2 <= dim_n, "");
    SG_ASSERT(i + dim_m_2 <= dim_m);
    SG_ASSERT(j + dim_n_2 <= dim_n);
    for(size_t ki = 0; ki < dim_m_2; ++ki)
    {
        for(size_t kj = 0; kj < dim_n_2; ++kj)
        {
            operator()(ki+i, kj+j) = iSubMatrix(ki,kj);
        }
    }
}
//=============================================================================
template<typename T, size_t dim, MatrixOrder order>
T Trace(Matrix<T, dim, dim, order> const& m)
{
    float trace = 0.f;
    for(size_t r = 0; r < dim; ++r)
    {
        trace += m(r,r);
    }
    return trace;
}
//=============================================================================
template<typename T, MatrixOrder order>
T Determinant(Matrix<T, 2, 2, order> const& m)
{
    T const m00 = m(0,0);
    T const m01 = m(0,1);
    T const m10 = m(1,0);
    T const m11 = m(1,1);
    T const det = m00 * m11 - m01 * m10;
    return det;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, MatrixOrder order>
T Determinant(Matrix<T, 3, 3, order> const& m)
{
    T const m00 = m(0,0);
    T const m01 = m(0,1);
    T const m02 = m(0,2);
    T const m10 = m(1,0);
    T const m11 = m(1,1);
    T const m12 = m(1,2);
    T const m20 = m(2,0);
    T const m21 = m(2,1);
    T const m22 = m(2,2);
    T const det =
          m00 * ( m11 * m22 - m21 * m12 )
        + m01 * ( m12 * m20 - m22 * m10 )
        + m02 * ( m10 * m21 - m20 * m11 );
#if SG_ENABLE_ASSERT
    T const det0 = det;
    T const det1 =
          m10 * ( m21 * m02 - m01 * m22 )
        + m11 * ( m22 * m00 - m02 * m20 )
        + m12 * ( m20 * m01 - m00 * m21 );
    T const det2 =
          m20 * ( m01 * m12 - m11 * m02 )
        + m21 * ( m02 * m10 - m12 * m00 )
        + m22 * ( m00 * m11 - m01 * m10 );
    T const refDet = 1.f / 3.f * (det0 + det1 + det2);
    SG_ASSERT(sg::EqualsWithTolerance(det, refDet, T(0.00001) * (det+refDet)));
#endif
    return det;
}
//=============================================================================
enum class MatrixInversionMethod { Default, Cramer, Cholesky, };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <MatrixInversionMethod method, typename T, size_t N, MatrixOrder order>
struct MatrixInversion {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, MatrixOrder order>
struct MatrixInversion<MatrixInversionMethod::Default, T, 2, order> : public MatrixInversion<MatrixInversionMethod::Cramer, T, 2, order> {};
template <typename T, MatrixOrder order>
struct MatrixInversion<MatrixInversionMethod::Default, T, 3, order> : public MatrixInversion<MatrixInversionMethod::Cramer, T, 3, order> {};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, MatrixOrder order>
struct MatrixInversion<MatrixInversionMethod::Cramer, T, 2, order>
{
    static_assert(std::is_floating_point<T>::value, "");
    typedef Matrix<T, 2, 2, order> matrix_type;
    matrix_type Invert_AssumeInvertible(matrix_type const& m)
    {
        T const m00 = m(0,0);
        T const m01 = m(0,1);
        T const m10 = m(1,0);
        T const m11 = m(1,1);
        T const det = m00 * m11 - m01 * m10;
        T const oodet = T(1) / det;
        matrix_type r;
        r(0,0) =  oodet * m11;
        r(0,1) = -oodet * m01;
        r(1,0) = -oodet * m10;
        r(1,1) =  oodet * m00;
        return r;
    }
    bool InvertROK(matrix_type& r, matrix_type const& m, T minDet)
    {
        T const m00 = m(0,0);
        T const m01 = m(0,1);
        T const m10 = m(1,0);
        T const m11 = m(1,1);
        T const det = m00 * m11 - m01 * m10;
        if(std::abs(det) <= minDet)
            return false;
        T const oodet = 1.f / det;
        r(0,0) =  oodet * m11;
        r(0,1) = -oodet * m01;
        r(1,0) = -oodet * m10;
        r(1,1) =  oodet * m00;
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, MatrixOrder order>
struct MatrixInversion<MatrixInversionMethod::Cramer, T, 3, order>
{
    static_assert(std::is_floating_point<T>::value, "");
    typedef Matrix<T, 3, 3, order> matrix_type;
    matrix_type Invert_AssumeInvertible(matrix_type const& m)
    {
        matrix_type r;
        bool ok = InvertROK(r, m, std::numeric_limits<T>::epsilon());
        SG_ASSERT_AND_UNUSED(ok);
        return r;
    }
    bool InvertROK(matrix_type& r, matrix_type const& m, T minDet)
    {
        T const m00 = m(0,0);
        T const m01 = m(0,1);
        T const m02 = m(0,2);
        T const m10 = m(1,0);
        T const m11 = m(1,1);
        T const m12 = m(1,2);
        T const m20 = m(2,0);
        T const m21 = m(2,1);
        T const m22 = m(2,2);

        matrix_type transposedCoMatrix(uninitialized);
        transposedCoMatrix(0,0) =  (m11 * m22 - m12 * m21);
        transposedCoMatrix(1,0) = -(m10 * m22 - m12 * m20);
        transposedCoMatrix(2,0) =  (m10 * m21 - m11 * m20);
        transposedCoMatrix(0,1) = -(m01 * m22 - m02 * m21);
        transposedCoMatrix(1,1) =  (m00 * m22 - m02 * m20);
        transposedCoMatrix(2,1) = -(m00 * m21 - m01 * m20);
        transposedCoMatrix(0,2) =  (m01 * m12 - m02 * m11);
        transposedCoMatrix(1,2) = -(m00 * m12 - m02 * m10);
        transposedCoMatrix(2,2) =  (m00 * m11 - m01 * m10);

        T const det0 = m00 * transposedCoMatrix(0,0) + m10 * transposedCoMatrix(0,1) + m20 * transposedCoMatrix(0,2);
        T const det = det0;
#if SG_ENABLE_ASSERT
        T const det1 = m01 * transposedCoMatrix(1,0) + m11 * transposedCoMatrix(1,1) + m21 * transposedCoMatrix(1,2);
        T const det2 = m02 * transposedCoMatrix(2,0) + m12 * transposedCoMatrix(2,1) + m22 * transposedCoMatrix(2,2);
        T const refDet = 1.f / 3.f * (det0 + det1 + det2);
        SG_ASSERT(sg::EqualsWithTolerance(det, refDet, T(0.00001) * (det+refDet)));
#endif
        if(std::abs(det) <= minDet)
            return false;
        T const oodet = 1.f / det;
        r = oodet * transposedCoMatrix;
        return true;
    }
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<MatrixInversionMethod method, typename T, size_t N, MatrixOrder order>
Matrix<T, N, N, order> Invert_AssumeInvertible(Matrix<T, N, N, order> const& m)
{
    MatrixInversion<method, T, N, order> f;
    return f.Invert_AssumeInvertible(m);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<MatrixInversionMethod method, typename T, size_t N, MatrixOrder order>
bool InvertROK(Matrix<T, N, N, order>& r, Matrix<T, N, N, order> const& m, T minDet)
{
    MatrixInversion<method, T, N, order> f;
    return f.InvertROK(r, m, minDet);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t N, MatrixOrder order>
Matrix<T, N, N, order> Invert_AssumeInvertible(Matrix<T, N, N, order> const& m)
{
    return Invert_AssumeInvertible<MatrixInversionMethod::Default>(m);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t N, MatrixOrder order>
bool InvertROK(Matrix<T, N, N, order>& r, Matrix<T, N, N, order> const& m, T minDet)
{
    return InvertROK<MatrixInversionMethod::Default>(r, m, minDet);
}
//=============================================================================
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
bool EqualsWithTolerance(Matrix<T, dim_m, dim_n, order> const& a, Matrix<T, dim_m, dim_n, order> const& b, Matrix<T, dim_m, dim_n, order> const& tolerance)
{
    bool r = true;
    for(size_t i = 0; i < dim_m; ++i)
        for(size_t j = 0; j < dim_n; ++j)
            r = r && sg::EqualsWithTolerance(a(i,j), b(i,j), tolerance(i,j));
    return r;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename T, size_t dim_m, size_t dim_n, MatrixOrder order>
bool EqualsWithTolerance(Matrix<T, dim_m, dim_n, order> const& a, Matrix<T, dim_m, dim_n, order> const& b, T tolerance)
{
    bool r = true;
    for(size_t i = 0; i < dim_m; ++i)
        for(size_t j = 0; j < dim_n; ++j)
            r = r && sg::EqualsWithTolerance(a(i,j), b(i,j), tolerance);
    return r;
}
//=============================================================================
#define Math_Matrix_Include_SpecialisedImpl
#include "Matrix_SpecialisedImpl.h"
#undef Math_Matrix_Include_SpecialisedImpl
//=============================================================================
} // namespace math
//=============================================================================
typedef math::Matrix<float, 2, 2, math::MatrixOrder::RowMajor> float2x2;
typedef math::Matrix<float, 3, 3, math::MatrixOrder::RowMajor> float3x3;
typedef math::Matrix<float, 4, 4, math::MatrixOrder::RowMajor> float4x4;
typedef math::Matrix<double, 2, 2, math::MatrixOrder::RowMajor> double2x2;
typedef math::Matrix<double, 3, 3, math::MatrixOrder::RowMajor> double3x3;
typedef math::Matrix<double, 4, 4, math::MatrixOrder::RowMajor> double4x4;
//=============================================================================
typedef math::Matrix<float, 2, 1, math::MatrixOrder::RowMajor> float2x1;
typedef math::Matrix<float, 1, 2, math::MatrixOrder::RowMajor> float1x2;
typedef math::Matrix<float, 3, 1, math::MatrixOrder::RowMajor> float3x1;
typedef math::Matrix<float, 1, 3, math::MatrixOrder::RowMajor> float1x3;
typedef math::Matrix<float, 4, 1, math::MatrixOrder::RowMajor> float4x1;
typedef math::Matrix<float, 1, 4, math::MatrixOrder::RowMajor> float1x4;
//=============================================================================
typedef math::Matrix<float, 2, 3, math::MatrixOrder::RowMajor> float2x3;
typedef math::Matrix<float, 3, 2, math::MatrixOrder::RowMajor> float3x2;
typedef math::Matrix<float, 3, 4, math::MatrixOrder::RowMajor> float3x4;
typedef math::Matrix<float, 4, 3, math::MatrixOrder::RowMajor> float4x3;
//=============================================================================
typedef math::Matrix<float, 2, 2, math::MatrixOrder::RowMajor> float2x2_rowmajor;
typedef math::Matrix<float, 3, 3, math::MatrixOrder::RowMajor> float3x3_rowmajor;
typedef math::Matrix<float, 4, 4, math::MatrixOrder::RowMajor> float4x4_rowmajor;
typedef math::Matrix<float, 2, 2, math::MatrixOrder::ColumnMajor> float2x2_colmajor;
typedef math::Matrix<float, 3, 3, math::MatrixOrder::ColumnMajor> float3x3_colmajor;
typedef math::Matrix<float, 4, 4, math::MatrixOrder::ColumnMajor> float4x4_colmajor;
//=============================================================================
namespace matrix {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline float2x2 Rotation(float iAngleInRadians);
inline float3x3 RotationX(float iAngleInRadians);
inline float3x3 RotationY(float iAngleInRadians);
inline float3x3 RotationZ(float iAngleInRadians);
inline float3x3 Rotation(float3 const& iAxis, float iAngleInRadians);
inline float3x3 Rotation_AssumeNormalisedAxis(float3 const& iAxis, float iAngleInRadians);
inline float4x4 HomogeneousRotationX(float iAngleInRadians);
inline float4x4 HomogeneousRotationY(float iAngleInRadians);
inline float4x4 HomogeneousRotationZ(float iAngleInRadians);
inline float4x4 HomogeneousRotation(float3 const& iAxis, float iAngleInRadians);
inline float4x4 HomogeneousRotation_AssumeNormalisedAxis(float3 const& iAxis, float iAngleInRadians);
inline float4x4 HomogeneousRotation(float4 const& iAxis, float iAngleInRadians);
inline float4x4 HomogeneousRotation_AssumeNormalisedAxis(float4 const& iAxis, float iAngleInRadians);
inline float4x4 HomogeneousTranslation(float3 const& iTranslation);
inline float4x4 HomogeneousTranslation(float4 const& iTranslation);
inline float4x4 HomogeneousScale(float iScale);
inline float4x4 HomogeneousScale(float3 const& iScale);
template<typename T, size_t dim> math::Matrix<T, dim, dim, math::MatrixOrder::RowMajor> Diagonal(math::Vector<T, dim> const& iDiagonal);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
}

//=============================================================================
#define Math_Matrix_Include_Impl
#include "Matrix_Impl.h"
#undef Math_Matrix_Include_Impl
//=============================================================================

#endif
