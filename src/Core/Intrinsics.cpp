#include "stdafx.h"

#include "Intrinsics.h"

#include "TestFramework.h"

#if SG_ENABLE_UNIT_TESTS

namespace sg {
//=============================================================================
SG_TEST((sg,core), Intrinsics, (quick))
{
    {
        //SG_ASSERT(1 == GetSmallestPowerOf2GreaterThan(0)); // this one should assert
        SG_ASSERT(1 == GetSmallestPowerOf2GreaterThan(1));
        SG_ASSERT(2 == GetSmallestPowerOf2GreaterThan(2));
        SG_ASSERT(4 == GetSmallestPowerOf2GreaterThan(3));
        SG_ASSERT(4 == GetSmallestPowerOf2GreaterThan(4));
        SG_ASSERT(8 == GetSmallestPowerOf2GreaterThan(5));
        SG_ASSERT(8 == GetSmallestPowerOf2GreaterThan(6));
        SG_ASSERT(8 == GetSmallestPowerOf2GreaterThan(7));
        SG_ASSERT(8 == GetSmallestPowerOf2GreaterThan(8));
        SG_ASSERT(64 == GetSmallestPowerOf2GreaterThan(63));
        SG_ASSERT(64 == GetSmallestPowerOf2GreaterThan(64));
        SG_ASSERT(128 == GetSmallestPowerOf2GreaterThan(65));
        SG_ASSERT(0x10000000 == GetSmallestPowerOf2GreaterThan(0x0BADCAFE));
    }
    {
        //SG_ASSERT(1 == GetLargestPowerOf2LessThan(0)); // this one should assert
        SG_ASSERT(1 == GetLargestPowerOf2LessThan(1));
        SG_ASSERT(2 == GetLargestPowerOf2LessThan(2));
        SG_ASSERT(2 == GetLargestPowerOf2LessThan(3));
        SG_ASSERT(4 == GetLargestPowerOf2LessThan(4));
        SG_ASSERT(4 == GetLargestPowerOf2LessThan(5));
        SG_ASSERT(4 == GetLargestPowerOf2LessThan(6));
        SG_ASSERT(4 == GetLargestPowerOf2LessThan(7));
        SG_ASSERT(8 == GetLargestPowerOf2LessThan(8));
        SG_ASSERT(32 == GetLargestPowerOf2LessThan(63));
        SG_ASSERT(64 == GetLargestPowerOf2LessThan(64));
        SG_ASSERT(64 == GetLargestPowerOf2LessThan(65));
        SG_ASSERT(0x08000000 == GetLargestPowerOf2LessThan(0x0BADCAFE));
    }
}
//=============================================================================
} // namespace sg

#endif
