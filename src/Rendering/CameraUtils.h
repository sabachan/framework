#ifndef Rendering_CameraUtils_H
#define Rendering_CameraUtils_H

#include <Math/Box.h>
#include <Math/Matrix.h>

namespace sg {
namespace rendering {
//=============================================================================
float4x4 ComputeProjectionMatrixFromTanHalfFovYNearFar(int2 const& iResolution, float iTanHalfFovY, float iNear, float iFar);
float4x4 ComputeProjectionMatrixFromBoxTanFovNearFar(box2f iBoxTanFovY, float iNear, float iFar);
float4x4 ComputInverseOfProjectionMatrix(float4x4 const& iProj);
float4x4 ComputeViewMatrixFromPosFrontUp(float3 const& iPos, float3 const& iFront, float3 const& iUp);
float4x4 ComputeViewMatrixFromPosAtUp(float3 const& iPos, float3 const& iAt, float3 const& iUp);
float4x4 ComputeInverseOfViewMatrix(float4x4 const& iView);
float3 GetFrontFromViewMatrix(float4x4 const& iView);
float3 GetUpFromViewMatrix(float4x4 const& iView);
float3 GetRigthFromViewMatrix(float4x4 const& iView);
//=============================================================================
}
}

#endif
