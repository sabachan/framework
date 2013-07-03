#include "stdafx.h"

#include "NumericalUtils.h"

#include "Vector.h"
#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/FixedPoint.h>
#include <Core/IntTypes.h>
#include <Core/TestFramework.h>

#if SG_ENABLE_UNIT_TESTS
namespace sg {
//=============================================================================
SG_TEST((sg), NumericalUtils, (Math, quick))
{
    typedef FixedPoint<i32, 16> fp32_16;

    {
        float const a = -10;
        float const b = -20;
        float r;
        r = fast::lerp(a, b, 0.31f);
        SG_ASSERT(EqualsWithTolerance(r, -13.1f, 0.0001f));
        r = fast::lerp(a, b, 0.78f);
        SG_ASSERT(EqualsWithTolerance(r, -17.8f, 0.0001f));

        r = symmetric::lerp(a, b, 0.31f);
        SG_ASSERT(EqualsWithTolerance(r, -13.1f, 0.0001f));
        r = symmetric::lerp(a, b, 0.78f);
        SG_ASSERT(EqualsWithTolerance(r, -17.8f, 0.0001f));
    }
    {
        float2 const a = float2(-10, 10);
        float2 const b = float2(-20, 20);
        float2 r;
        r = fast::lerp(a, b, 0.31f);
        SG_ASSERT(EqualsWithTolerance(r, float2(-13.1f, 13.1f), 0.0001f));
        r = symmetric::lerp(a, b, 0.31f);
        SG_ASSERT(EqualsWithTolerance(r, float2(-13.1f, 13.1f), 0.0001f));

        r = fast::lerp(a, b, float2(0.43f, 0.78f));
        SG_ASSERT(EqualsWithTolerance(r, float2(-14.3f, 17.8f), 0.0001f));
        r = symmetric::lerp(a, b, float2(0.43f, 0.78f));
        SG_ASSERT(EqualsWithTolerance(r, float2(-14.3f, 17.8f), 0.0001f));
    }
    {
        i32 const a = 10;
        i32 const b = 20;
        i32 r;
        r = i32(fast::lerp(a, b, 0.31f));
        SG_ASSERT(r == 13);
        r = i32(fast::lerp(a, b, fp32_16(0.31f)));
        SG_ASSERT(r == 13);
        r = i32(fast::lerp(a, b, 0.78f));
        SG_ASSERT(r == 17);
        r = roundi(fast::lerp(a, b, 0.78f));
        SG_ASSERT(r == 18);
        r = i32(fast::lerp(a, b, fp32_16(0.78f)));
        SG_ASSERT(r == 17);
        r = roundi(fast::lerp(a, b, fp32_16(0.78f)));
        SG_ASSERT(r == 18);

        r = i32(symmetric::lerp(a, b, 0.31f));
        SG_ASSERT(r == 13);
        r = i32(symmetric::lerp(a, b, fp32_16(0.31f)));
        SG_ASSERT(r == 13);
        r = i32(symmetric::lerp(a, b, 0.78f));
        SG_ASSERT(r == 17);
        r = roundi(symmetric::lerp(a, b, 0.78f));
        SG_ASSERT(r == 18);
        r = i32(symmetric::lerp(a, b, fp32_16(0.78f)));
        SG_ASSERT(r == 17);
        r = roundi(symmetric::lerp(a, b, fp32_16(0.78f)));
        SG_ASSERT(r == 18);
    }
}
//=============================================================================
}
#endif

