#include "stdafx.h"

#include "CameraUtils.h"


namespace sg {
namespace rendering {
//=============================================================================
// Note about orientations:
// I try to work with all frames direct oriented, and front faces that are
// counter clockwise.
// Render target should have its x axis to the right and z pointing front,
// which should implies a y pointing down.
// However, as DirectX uses a y pointing up, it imposes an indirect frame for
// render target.
// We will work as if y were downside, and flip it as the last step.
//=============================================================================
float4x4 ComputeProjectionMatrixFromTanHalfFovYNearFar(int2 const& iResolution, float iTanHalfFovY, float iNear, float iFar)
{
    float2 const viewportWH = float2(iResolution);
    float const aspectRatio = viewportWH.x() / viewportWH.y();


    float const fovYMin = -iTanHalfFovY; // y / z at the boundary of viewport = tan(a/2), where a is the angle of vision
    float const fovYMax = iTanHalfFovY;
    float const fovXMin = fovYMin * aspectRatio;
    float const fovXMax = fovYMax * aspectRatio;

    return ComputeProjectionMatrixFromBoxTanFovNearFar(box2f::FromMinMax(float2(fovXMin, fovYMin), float2(fovXMax, fovYMax)), iNear, iFar);
}

float4x4 ComputeProjectionMatrixFromBoxTanFovNearFar(box2f iBoxTanFovY, float iNear, float iFar)
{
    float const fovYMin = iBoxTanFovY.Min().y();
    float const fovYMax = iBoxTanFovY.Max().y();
    float const fovXMin = iBoxTanFovY.Min().x();
    float const fovXMax = iBoxTanFovY.Max().x();

    // Assuming a viewing directon along z, x to the right, y to the bottom, with
    // the center of projection in (0,0).
    // We look for matrix Proj such that:
    // X' = Proj . X
    // if X E Frustum, X' E [-1,1] x [-1,1] x [0,1]
    // The matrix:
    //              1   0   0   0
    //    Proj =    0   1   0   0
    //              0   0   1   0
    //              0   0   1   0
    // gives X' = (x/z, y/z, z/z) = (x/z, y/z, 1)
    // The matrix:
    //              1   0   0   0
    //    Proj =    0   1   0   0
    //              0   0   0   1
    //              0   0   1   0
    // gives X' = (x/z, y/z, 1/z)

    float const np = iNear;
    float const fp = iFar;

    float const ooAvgFovX = 2.f/(fovXMax-fovXMin);
    float const ooAvgFovY = 2.f/(fovYMax-fovYMin);
    float const fovOffsetX = -(fovXMax+fovXMin)/(fovXMax-fovXMin);
    float const fovOffsetY = -(fovYMax+fovYMin)/(fovYMax-fovYMin);

    float const values[] = {
        ooAvgFovX,  0,         fovOffsetX,  0,
        0,          ooAvgFovY, fovOffsetY,  0,
        0,          0,         fp/(fp-np), -fp*np/(fp-np),
        0,          0,         1.f,         0,
    };
    float4x4 const preproj = float4x4(values);

    float const revertYValues[] = {
        1,  0,  0,  0,
        0, -1,  0,  0,
        0,  0,  1,  0,
        0,  0,  0,  1,
    };
    float4x4 const revertY = float4x4(revertYValues);
    float4x4 const proj = revertY * preproj;

    return proj;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4x4 ComputInverseOfProjectionMatrix(float4x4 const& iProj)
{
    float invProjValues[] = {
        1.f/iProj(0,0),  0,               0.f,             -iProj(0,2) / iProj(0,0),
        0,               1.f/iProj(1,1),  0.f,             -iProj(1,2) / iProj(1,1),
        0,               0,               0.f,              1.f,
        0,               0,               1.f/iProj(2,3),  -iProj(2,2) / iProj(2,3),
    };
    float4x4 const invProj = float4x4(invProjValues);

#if SG_ENABLE_ASSERT
    float4x4 dbg = invProj * iProj;
    float4x4 dbg2 = iProj * invProj;
    SG_ASSERT(EqualsWithTolerance(dbg,  float4x4::Identity(), 0.001f));
    SG_ASSERT(EqualsWithTolerance(dbg2, float4x4::Identity(), 0.001f));
#endif

    return invProj;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4x4 ComputeViewMatrixFromPosFrontUp(float3 const& iPos, float3 const& iFront, float3 const& iUp)
{
    float3 const ez = iFront.Normalised();
    float3 const preex = cross(iFront, iUp);
    float const normSqPreex = preex.LengthSq();
    SG_ASSERT(normSqPreex > ez.LengthSq() * 0.000001);
    float3 const ex = preex.Normalised();
    float3 const ey = cross(ez, ex);
    float3x3 R;
    R.SetRow(0, ex);
    R.SetRow(1, ey);
    R.SetRow(2, ez);
    float3 const t = -(R * iPos);
    float4x4 view;
    view.SetSubMatrix(0,0,R);
    view.SetCol(3, t.Append(1));
    view.SetRow(3, float4(0,0,0,1));
    return view;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4x4 ComputeViewMatrixFromPosAtUp(float3 const& iPos, float3 const& iAt, float3 const& iUp)
{
    return ComputeViewMatrixFromPosFrontUp(iPos, iAt - iPos, iUp);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float4x4 ComputeInverseOfViewMatrix(float4x4 const& iView)
{
    float3x3 const R = iView.SubMatrix<3,3>(0,0);
    float3x3 const invR = R.Transposed();
    float3 const T = iView.Col(3).xyz();
    float3 const invT = invR * -T;
    float4x4 invView;
    invView.SetSubMatrix(0, 0, invR);
    invView.SetCol(3, invT.xyz1());
    invView.SetRow(3, float4(0,0,0,1));

#if SG_ENABLE_ASSERT
    float4x4 dbg = invView * iView;
    float4x4 dbg2 = iView * invView;
    SG_ASSERT(EqualsWithTolerance(dbg,  float4x4::Identity(), 0.001f));
    SG_ASSERT(EqualsWithTolerance(dbg2, float4x4::Identity(), 0.001f));
#endif

    return invView;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float3 GetFrontFromViewMatrix(float4x4 const& iView)
{
    float3 const ez = iView.Row(2).xyz();
    SG_ASSERT(EqualsWithTolerance(ez.LengthSq(), 1.f, 0.0001f));
    return ez;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float3 GetUpFromViewMatrix(float4x4 const& iView)
{
    float3 const ey = iView.Row(1).xyz();
    SG_ASSERT(EqualsWithTolerance(ey.LengthSq(), 1.f, 0.0001f));
    return -ey;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float3 GetRigthFromViewMatrix(float4x4 const& iView)
{
    float3 const ex = iView.Row(0).xyz();
    SG_ASSERT(EqualsWithTolerance(ex.LengthSq(), 1.f, 0.0001f));
    return ex;
}
//=============================================================================
}
}
