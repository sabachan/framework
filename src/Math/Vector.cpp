#include "stdafx.h"

#include "Vector.h"
#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/TestFramework.h>

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace math {
//=============================================================================
SG_TEST((sg, math), Vector, (Math, quick))
{
    {
        float2 v2;
        SG_ASSERT(0.f == v2.x());
        SG_ASSERT(0.f == v2.y());
        v2.x() = 3.f;
        v2.y() = 4.f;
        float3 v3 = v2.xyx();
        SG_ASSERT(3.f == v3.x());
        SG_ASSERT(4.f == v3.y());
        SG_ASSERT(3.f == v3.z());
        float4 v4 = v3.y001();
        SG_ASSERT(4.f == v4.x());
        SG_ASSERT(0.f == v4.y());
        SG_ASSERT(0.f == v4.z());
        SG_ASSERT(1.f == v4.w());
    }
    {
        float4 const a(1,2,3,4);
        float4 const one(1);
        float4 r = a + one;
        SG_ASSERT(2.f == r.x());
        SG_ASSERT(3.f == r.y());
        SG_ASSERT(4.f == r.z());
        SG_ASSERT(5.f == r.w());
        r -= one;
        SG_ASSERT(a == r);
        r *= 2.f;
        SG_ASSERT(2.f == r.x());
        SG_ASSERT(4.f == r.y());
        SG_ASSERT(6.f == r.z());
        SG_ASSERT(8.f == r.w());
        float4 b(2,2,2,2);
        r = (a / b) + one;
        SG_ASSERT(1.5f == r.x());
        SG_ASSERT(2.f  == r.y());
        SG_ASSERT(2.5f == r.z());
        SG_ASSERT(3.f  == r.w());
    }
    {
        float3 a(1,0,0);
        float3 b(1,1,0);
        float dotab = dot(a,b);
        SG_ASSERT_AND_UNUSED(1.f == dotab);
        float3 crossab = cross(a,b);
        SG_ASSERT(0.f == crossab.x());
        SG_ASSERT(0.f == crossab.y());
        SG_ASSERT(1.f == crossab.z());
    }
    {
        float3 a(1.1f, -1.5f, 1.9f);
        int3 b = floori(a);
        SG_ASSERT(int3(1,-2,1) == b);
        int3 c = ceili(a);
        SG_ASSERT(int3(2,-1,2) == c);
        int3 d = roundi(a);
        SG_ASSERT(int3(1,-2,2) == d);
    }
    {
        uint4 v(0);
        SG_ASSERT(uint4(0,0,0,0) == v);
        v.x() = 1;
        SG_ASSERT(uint4(1,0,0,0) == v);
        v.xy() = uint2(3,4);
        SG_ASSERT(uint4(3,4,0,0) == v);
        v.wz() = uint4(1,2,3,4).xz();
        SG_ASSERT(uint4(3,4,3,1) == v);
        uint2 u = v.wy();
        SG_ASSERT(uint2(1,4) == u);
        v.zxwy() = uint4(1,2,3,4);
        SG_ASSERT(uint4(2,4,1,3) == v);
        v.xyz() = uint4(1,2,3,4).wxy();
        SG_ASSERT(uint4(4,1,2,3) == v);
        //v.xyx() = uint3(1,2,3); // must fail
    }
    {
        float4 a(1,2,3,4);
        float4 b(6,5,4,3);
        float4 c(5,5,5,5);
        SG_ASSERT( any(a < b));
        SG_ASSERT( all(a < c));
        SG_ASSERT( any(b < c));
        SG_ASSERT(!all(b < c));
    }
    {
        float4 v(1,2,3,4);
        float2 const a = v.SubVector<2>(1);
        SG_ASSERT(float2(2,3) == a);
        v.SetSubVector(2,float2(5,6));
        SG_ASSERT(float4(1,2,5,6) == v);
        Vector<float, 5> const b = v.Append(7);
        Vector<float, 6> const c = v.Append(float2(7, 8));
        SG_ASSERT(c.SubVector<5>(0) == b);
    }
}
//=============================================================================
}
}
#endif

