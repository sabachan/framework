#include "stdafx.h"

#include "FixedPoint.h"

#if SG_ENABLE_UNIT_TESTS
#include "IntTypes.h"
#include "TestFramework.h"

namespace sg {
//=============================================================================
SG_TEST((sg), FixedPoint, (Core, quick))
{
    typedef FixedPoint<i32, 8> fp32_8;
    typedef FixedPoint<i32, 16> fp32_16;

    {
        fp32_8 const a = fp32_8(3) / 7;
        fp32_8 const b = fp32_8(6) / 7;
        fp32_8 const c = 4 * a + 2 * b;
        i32 const d = floori(c);
        i32 const e = roundi(c);
        i32 const f = ceili(c);
        SG_ASSERT_AND_UNUSED(d <= e);
        SG_ASSERT_AND_UNUSED(e <= f);
        SG_ASSERT_AND_UNUSED(d == 3);
        SG_ASSERT_AND_UNUSED(e == 3);
        SG_ASSERT_AND_UNUSED(f == 4);
    }
    {
        fp32_8 const a = -fp32_8(3) / 7;
        fp32_8 const b = fp32_8(6) / 7;
        fp32_8 const c = 4 * a - 2 * b;
        i32 const d = floori(c);
        i32 const e = roundi(c);
        i32 const f = ceili(c);
        SG_ASSERT_AND_UNUSED(d <= e);
        SG_ASSERT_AND_UNUSED(e <= f);
        SG_ASSERT_AND_UNUSED(d ==-4);
        SG_ASSERT_AND_UNUSED(e == -3);
        SG_ASSERT_AND_UNUSED(f == -3);
    }
    {
        fp32_8 const a = fp32_8(23) / 7;
        fp32_8 const b = fp32_8(8) / 3;
        fp32_16 const c = a * b;
        i32 const d = floori(c);
        i32 const e = roundi(c);
        i32 const f = ceili(c);
        SG_ASSERT_AND_UNUSED(d <= e);
        SG_ASSERT_AND_UNUSED(e <= f);
        SG_ASSERT_AND_UNUSED(d == 8);
        SG_ASSERT_AND_UNUSED(e == 9);
        SG_ASSERT_AND_UNUSED(f == 9);
        fp32_16 const g = floor(c);
        fp32_16 const h = round(c);
        fp32_16 const i = ceil(c);
        SG_ASSERT_AND_UNUSED(g == fp32_16(d));
        SG_ASSERT_AND_UNUSED(h == fp32_16(e));
        SG_ASSERT_AND_UNUSED(i == fp32_16(f));
        i32 const j = i32(g);
        i32 const k = i32(h);
        i32 const l = i32(i);
        SG_ASSERT_AND_UNUSED(j == d);
        SG_ASSERT_AND_UNUSED(k == e);
        SG_ASSERT_AND_UNUSED(l == f);
    }
}
//=============================================================================
}
#endif
