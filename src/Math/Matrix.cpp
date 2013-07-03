#include "stdafx.h"

#include "Matrix.h"

#include "NumericalUtils.h"
#include <Core/Assert.h>
#include <Core/TestFramework.h>
#include <limits>

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace math {
//=============================================================================
namespace {
    void TestMatrixOrder()
    {
        float data[] =
        {
            0.5f, 0.3f, 0.1f,
            -0.1f, 0.6f, -0.1f
        };
        Matrix<float, 2, 3, MatrixOrder::RowMajor> const M(data);
        float3 const p(1.f, 0.5f, 0.2f);
        float2 const ref = M * p;
        Matrix<float, 2, 3, MatrixOrder::ColumnMajor> const Mc = M;
        float2 const res = Mc * p;
        SG_ASSERT_AND_UNUSED(ref == res);
    }
    void TestRotationMatrices()
    {
        SG_CODE_FOR_ASSERT(float tolerance = std::numeric_limits<float>::epsilon() * 10.f;)
        {
            float2x2 m = matrix::Rotation(float(math::constant::PI_2));
            float2 v = float2(1,0);
            float2 r = m * v;
            SG_ASSERT(EqualsWithTolerance(float2(0,1), r, tolerance));
            v = float2(0,1);
            r = m * v;
            SG_ASSERT(EqualsWithTolerance(float2(-1,0), r, tolerance));
            v = float2(1,1);
            r = m * v;
            SG_ASSERT(EqualsWithTolerance(float2(-1,1), r, tolerance));
        }
        {
            float const angle = 0.1f;
            float2x2 m2 = matrix::Rotation(angle);
            float3x3 m3X = matrix::RotationX(angle);
            float3x3 m3Y = matrix::RotationY(angle);
            float3x3 m3Z = matrix::RotationZ(angle);
            float4x4 mhX = matrix::HomogeneousRotationX(angle);
            float4x4 mhY = matrix::HomogeneousRotationY(angle);
            float4x4 mhZ = matrix::HomogeneousRotationZ(angle);
            float2 v2 = float2(3.f,2.f);
            float3 v3X = float3(1.f,3.f,2.f);
            float3 v3Y = float3(2.f,1.f,3.f);
            float3 v3Z = float3(3.f,2.f,1.f);
            float4 vhX = float4(1.f,3.f,2.f,1.f);
            float4 vhY = float4(2.f,1.f,3.f,1.f);
            float4 vhZ = float4(3.f,2.f,1.f,0.f);
            float2 r2 = m2 * v2;
            float3 r3X = m3X * v3X;
            float3 r3Y = m3Y * v3Y;
            float3 r3Z = m3Z * v3Z;
            float4 rhX = mhX * vhX;
            float4 rhY = mhY * vhY;
            float4 rhZ = mhZ * vhZ;
            SG_ASSERT(r3X.x() == v3X.x());
            SG_ASSERT(r3Y.y() == v3Y.y());
            SG_ASSERT(r3Z.z() == v3Z.z());
            SG_ASSERT(rhX.x() == vhX.x());
            SG_ASSERT(rhY.y() == vhY.y());
            SG_ASSERT(rhZ.z() == vhZ.z());
            SG_ASSERT(rhX.w() == vhX.w());
            SG_ASSERT(rhY.w() == vhY.w());
            SG_ASSERT(rhZ.w() == vhZ.w());
            SG_ASSERT(EqualsWithTolerance(r2, r3X.yz(), tolerance));
            SG_ASSERT(EqualsWithTolerance(r2, r3Y.zx(), tolerance));
            SG_ASSERT(EqualsWithTolerance(r2, r3Z.xy(), tolerance));
            SG_ASSERT(EqualsWithTolerance(r2, rhX.yz(), tolerance));
            SG_ASSERT(EqualsWithTolerance(r2, rhY.zx(), tolerance));
            SG_ASSERT(EqualsWithTolerance(r2, rhZ.xy(), tolerance));
        }
        {
            float angle = -2.f;
            float3x3 m1 = matrix::RotationX(angle);
            float3x3 m2 = matrix::Rotation_AssumeNormalisedAxis(float3(1,0,0), angle);
            float4x4 m3 = matrix::HomogeneousRotation_AssumeNormalisedAxis(float3(-1,0,0), -angle);
            float3 v = float3(-0.3f, 1.2f, 0.4f);
            float3 r1 = m1 * v;
            float3 r2 = m2 * v;
            float4 r3 = m3 * v.xyz1();
            SG_ASSERT(EqualsWithTolerance(r1, r2, tolerance));
            SG_ASSERT(EqualsWithTolerance(r1, r3.xyz(), tolerance));
        }
        {
            float angle = 1.5f;
            float3x3 m1 = matrix::RotationY(angle);
            float3x3 m2 = matrix::Rotation_AssumeNormalisedAxis(float3(0,1,0), angle);
            float4x4 m3 = matrix::HomogeneousRotation_AssumeNormalisedAxis(float3(0,-1,0), -angle);
            float3 v = float3(-0.3f, 1.2f, 0.4f);
            float3 r1 = m1 * v;
            float3 r2 = m2 * v;
            float4 r3 = m3 * v.xyz1();
            SG_ASSERT(EqualsWithTolerance(r1, r2, tolerance));
            SG_ASSERT(EqualsWithTolerance(r1, r3.xyz(), tolerance));
        }
        {
            float angle = 0.4f;
            float3x3 m1 = matrix::RotationZ(angle);
            float3x3 m2 = matrix::Rotation_AssumeNormalisedAxis(float3(0,0,-1), -angle);
            float4x4 m3 = matrix::HomogeneousRotation_AssumeNormalisedAxis(float3(0,0,1), angle);
            float3 v = float3(-0.3f, 1.2f, 0.4f);
            float3 r1 = m1 * v;
            float3 r2 = m2 * v;
            float4 r3 = m3 * v.xyz1();
            SG_ASSERT(EqualsWithTolerance(r1, r2, tolerance));
            SG_ASSERT(EqualsWithTolerance(r1, r3.xyz(), tolerance));
        }
    }
    void TesMatrixInversion()
    {
        {
            float m_values[] = {
                0.5f,   0.25f,
                0.f,    1.f,
            };
            float2x2 const m(m_values);
            float2x2 const invm = Invert_AssumeInvertible(m);
            float2x2 const m_invm = m * invm;
            SG_ASSERT(EqualsWithTolerance(m_invm, float2x2::Identity(), 0.000001f));
        }
        {
            float m_values[] = {
                0.5f,   0.25f,  0.f,
                0.25f,  1.f,    0.f,
                0.f,    0.75f,  1.f,
            };
            float3x3 const m(m_values);
            float3x3 const invm = Invert_AssumeInvertible(m);
            float3x3 const m_invm = m * invm;
            SG_ASSERT(EqualsWithTolerance(m_invm, float3x3::Identity(), 0.000001f));
        }
    }
}
//=============================================================================
SG_TEST((sg, math), Matrix, (Math, quick))
{
    {
        float2x2 m1;
        SG_ASSERT(0.f == m1(0,0));
        SG_ASSERT(0.f == m1(1,0));
        SG_ASSERT(0.f == m1(0,1));
        SG_ASSERT(0.f == m1(1,1));
        m1(0,0) = 1.f;
        m1(1,0) = 1.f;
        m1(0,1) = 0.f;
        m1(1,1) = 1.f;
        float2x2 m2;
        m2(0,0) = 1.f;
        m2(1,0) = 0.f;
        m2(0,1) = 1.f;
        m2(1,1) = 1.f;
        float2x2 m3 = m1 * m2;
        SG_ASSERT(1.f == m3(0,0));
        SG_ASSERT(1.f == m3(1,0));
        SG_ASSERT(1.f == m3(0,1));
        SG_ASSERT(2.f == m3(1,1));
        float2x2 m4 = m2 * m1;
        SG_ASSERT(2.f == m4(0,0));
    }
    {
        typedef Matrix<float, 2, 3, MatrixOrder::RowMajor> float2x3_rm;
        typedef Matrix<float, 2, 3, MatrixOrder::ColumnMajor> float2x3_cm;
        float m_values[] = {
            0.5f,   0.5f,   0.f,
            0.f,    1.f,    0.5f
        };
        float2x3_rm m(m_values);
        float3 r3;
        float2 r2;
        r3 = m.Row(0);
        SG_ASSERT(float3(0.5f, 0.5f, 0.f) == r3);
        r3 = m.Row(1);
        SG_ASSERT(float3(0.f, 1.f, 0.5f) == r3);
        r2 = m.Col(0);
        SG_ASSERT(float2(0.5f, 0.f) == r2);
        r2 = m.Col(1);
        SG_ASSERT(float2(0.5f, 1.f) == r2);
        r2 = m.Col(2);
        SG_ASSERT(float2(0.f, 0.5f) == r2);
        float2x3_cm m2 = m;
        r3 = m.Row(0);
        SG_ASSERT(float3(0.5f, 0.5f, 0.f) == r3);
        r3 = m.Row(1);
        SG_ASSERT(float3(0.f, 1.f, 0.5f) == r3);
        r2 = m.Col(0);
        SG_ASSERT(float2(0.5f, 0.f) == r2);
        r2 = m.Col(1);
        SG_ASSERT(float2(0.5f, 1.f) == r2);
        r2 = m.Col(2);
        SG_ASSERT(float2(0.f, 0.5f) == r2);
        float2x2 sub = m.SubMatrix<2,2>(0,1);
        SG_ASSERT(float2(0.5f, 0.f) == sub.Row(0));
        SG_ASSERT(float2(1.f,  0.5f) == sub.Row(1));
        m.SetSubMatrix(0,0,sub);
        SG_ASSERT(float3(0.5f, 0.f, 0.f) == m.Row(0));
        SG_ASSERT(float3(1.f,  0.5f, 0.5f) == m.Row(1));
    }
    {
        typedef Matrix<float, 2, 3, MatrixOrder::RowMajor> float2x3_rm;
        typedef Matrix<float, 3, 2, MatrixOrder::ColumnMajor> float3x2_cm;
        float2x3_rm m1;
        float3x2_cm m2;
        m1.SetRow(0, float3(1.f, 0.f, 1.f));
        m1.SetRow(1, float3(0.f, 1.f, 1.f));
        m2.SetRow(0, float2(1.f, 0.f));
        m2.SetRow(1, float2(-1.f, 0.f));
        m2.SetRow(2, float2(1.f, 1.f));
        float2x2 m3 = m1 * m2;
        SG_ASSERT(float2(2.f, 1.f) == m3.Row(0));
        SG_ASSERT(float2(0.f, 1.f) == m3.Row(1));
        float3x3 m4 = m2 * m1;
        SG_ASSERT(float3(1.f, 0.f, 1.f) == m4.Row(0));
        SG_ASSERT(float3(-1.f, 0.f, -1.f) == m4.Row(1));
        SG_ASSERT(float3(1.f, 1.f, 2.f) == m4.Row(2));
    }
    {
        float m_values[] = {
            0.5f,   0.5f,   0.f,
            0.f,    1.f,    0.5f,
            0.f,    0.f,    2.f,
        };
        float3x3 m(m_values);
        float3 v1(1,0,0);
        float3 v2(0,1,1);
        float3 r;
        r = m * v1;
        SG_ASSERT(float3(0.5f, 0.f, 0.f) == r);
        r = m * v2;
        SG_ASSERT(float3(0.5f, 1.5f, 2.f) == r);
        float s = v1 * m * v2;
        SG_ASSERT_AND_UNUSED(0.5f == s);
        m = m.Transposed();
        r = m * v1;
        SG_ASSERT(float3(0.5f, 0.5f, 0.f) == r);
        r = m * v2;
        SG_ASSERT(float3(0.f, 1.f, 2.5f) == r);
    }
    {
        float4x4 m = matrix::Diagonal(float4(2.f, 1.f, 0.5f, 0.f));
        SG_ASSERT(m(0,0) == 2.f);
        SG_ASSERT(m(1,1) == 1.f);
        SG_ASSERT(m(2,2) == 0.5f);
        SG_ASSERT(m(3,3) == 0.f);
        SG_ASSERT(m(0,3) == 0.f);
        SG_ASSERT(m(2,1) == 0.f);
    }
    TestRotationMatrices();
    TestMatrixOrder();
    TesMatrixInversion();
}
//=============================================================================
}
}
#endif
