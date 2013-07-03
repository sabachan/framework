#include "stdafx.h"

#include "Quaternion.h"

#include "NumericalUtils.h"
#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/TestFramework.h>


#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace math {
//=============================================================================
SG_TEST((sg, math), Quaternion, (Math, quick))
{
    SG_CODE_FOR_ASSERT(float tolerance = std::numeric_limits<float>::epsilon() * 10.f;)
    {
        float3x3 m0 = matrix::Rotation(float3(1.f,0.5f,0.f), 0.5f);
        quaternion q = quaternion::RepresentRotation(float3(1.f,0.5f,0.f), 0.5f);
        float3x3 m1 = matrix::Rotation(q);
        float3 v = float3(-0.3f, 1.2f, 0.4f);
        float3 r0 = m0 * v;
        float3 r1 = m1 * v;
        SG_ASSERT(EqualsWithTolerance(r0, r1, tolerance));
    }

    {
        quaternion q0 = quaternion::RepresentRotation(float3(1,0,0), float(math::constant::PI_2));
        quaternion q1 = quaternion::RepresentRotation(float3(0,1,0), float(math::constant::PI_2));
        quaternion q = q1 * q0;
        float3x3 m = matrix::Rotation(q);
        float3 rx = m * float3(1,0,0);
        float3 ry = m * float3(0,1,0);
        float3 rz = m * float3(0,0,1);
        SG_ASSERT(EqualsWithTolerance(rx, float3(0,0,-1), tolerance));
        SG_ASSERT(EqualsWithTolerance(ry, float3(1,0,0),  tolerance));
        SG_ASSERT(EqualsWithTolerance(rz, float3(0,-1,0), tolerance));
    }
}
//=============================================================================
}
}
#endif
