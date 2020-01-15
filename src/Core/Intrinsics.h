#ifndef Core_Intrinsics_H
#define Core_Intrinsics_H

#include "Assert.h"

#include <immintrin.h>
#include <limits>

namespace sg {
//=============================================================================
SG_FORCE_INLINE size_t CeilLog2(size_t i)
{
    SG_CODE_FOR_ASSERT(size_t const maxAcceptableValue = (std::numeric_limits<size_t>::max() >> 1) + 1;)
    SG_ASSERT(i <= maxAcceptableValue);
    SG_ASSERT(i != 0);
#if SG_ARCH_IS_X86 || SG_ARCH_IS_X64
    DWORD index;
#if SG_ARCH_IS_X86
    static_assert(sizeof(size_t) == 4, "");
    BOOLEAN const isNotZero = _BitScanReverse(&index, i-1);
#elif SG_ARCH_IS_X64
    static_assert(sizeof(size_t) == 8, "");
    BOOLEAN const isNotZero = _BitScanReverse64(&index, i-1);
#endif
    return isNotZero ? index+1 : 0;
#else
    i = i - 1;
    size_t j = 0;
    while(i != 0)
    {
        i = i >> 1;
        j += 1;
    }
    return j;
#endif
}
SG_FORCE_INLINE size_t FloorLog2(size_t i)
{
    SG_CODE_FOR_ASSERT(size_t const maxAcceptableValue = (std::numeric_limits<size_t>::max() >> 1) + 1;)
    SG_ASSERT(i <= maxAcceptableValue);
    SG_ASSERT(i != 0);
#if SG_ARCH_IS_X86 || SG_ARCH_IS_X64
    DWORD index;
#if SG_ARCH_IS_X86
    static_assert(sizeof(size_t) == 4, "");
    BOOLEAN const isNotZero = _BitScanReverse(&index, i);
#elif SG_ARCH_IS_X64
    static_assert(sizeof(size_t) == 8, "");
    BOOLEAN const isNotZero = _BitScanReverse64(&index, i);
#endif
    return index;
#else
    i = i - 1;
    size_t j = 0;
    while(i != 0)
    {
        i = i >> 1;
        j += 1;
    }
    return j;
#endif
}
//=============================================================================
SG_FORCE_INLINE size_t GetSmallestPowerOf2GreaterThan(size_t i)
{
    SG_CODE_FOR_ASSERT(size_t const maxAcceptableValue = (std::numeric_limits<size_t>::max() >> 1) + 1;)
    SG_ASSERT(i <= maxAcceptableValue);
    SG_ASSERT(i != 0);
#if 1
    return size_t(1) << CeilLog2(i);
#else
#if SG_ARCH_IS_X86 || SG_ARCH_IS_X64
    DWORD index;
#if SG_ARCH_IS_X86
    static_assert(sizeof(size_t) == 4, "");
    BOOLEAN const isNotZero = _BitScanReverse(&index, i-1);
#elif SG_ARCH_IS_X64
    static_assert(sizeof(size_t) == 8, "");
    BOOLEAN const isNotZero = _BitScanReverse64(&index, i-1);
#endif
    return isNotZero ? size_t(1) << (index+1) : 1;
#else
    i = i - 1;
    size_t j = 1;
    while(i != 0)
    {
        i = i >> 1;
        j = j << 1;
    }
    return j;
#endif
#endif
}
SG_FORCE_INLINE size_t GetLargestPowerOf2LessThan(size_t i)
{
    SG_CODE_FOR_ASSERT(size_t const maxAcceptableValue = (std::numeric_limits<size_t>::max() >> 1) + 1;)
    SG_ASSERT(i <= maxAcceptableValue);
    SG_ASSERT(i != 0);
#if 1
    return size_t(1) << FloorLog2(i);
#else
#if SG_ARCH_IS_X86 || SG_ARCH_IS_X64
    DWORD index;
#if SG_ARCH_IS_X86
    static_assert(sizeof(size_t) == 4, "");
    BOOLEAN const isNotZero = _BitScanReverse(&index, i);
#elif SG_ARCH_IS_X64
    static_assert(sizeof(size_t) == 8, "");
    BOOLEAN const isNotZero = _BitScanReverse64(&index, i);
#endif
    SG_ASSERT(isNotZero);
    return size_t(1) << index;
#else
    size_t j = 1;
    while(i != 0)
    {
        i = i >> 1;
        j = j << 1;
    }
    return j >> 1;
#endif
#endif
}
//=============================================================================
SG_FORCE_INLINE u64 GetPopCount(u16 i)
{
#if SG_ARCH_IS_X64
    return __popcnt16(i);
#else
#error "not implemented"
#endif
}
SG_FORCE_INLINE u32 GetPopCount(u32 i)
{
#if SG_ARCH_IS_X64
    return __popcnt(i);
#else
#error "not implemented"
#endif
}
SG_FORCE_INLINE u64 GetPopCount(u64 i)
{
#if SG_ARCH_IS_X64
    return __popcnt64(i);
#else
#error "not implemented"
#endif
}
//=============================================================================
} // namespace sg

#endif
