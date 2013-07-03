#include "stdafx.h"

#include "BitMapFont.h"

#include <Core/For.h>

namespace sg {
namespace image {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int CompareCharCode(u8 const* a, u8 const* b, size_t sizeInBytes)
{
    u8 const* a_end = a + sizeInBytes;
    do
    {
        int r = *a-*b;
        if(0 != r) return r;
        ++a;
        ++b;
    } while(a != a_end);
    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
void BitMapFont::GetGlyphInfo(GlyphInfo& oInfo, u32 iCharCode) const
{
    u8 const* const charCodesAsu8 = reinterpret_cast<u8 const*>(charCodes);
#if SG_ENABLE_ASSERT
    u8 const zeroAsu8[] = {0,0,0,0};
    SG_ASSERT(0 == CompareCharCode(zeroAsu8, charCodesAsu8, charCodeSizeInBytes));
#endif
    u32 const maxCharCode = (1 << (charCodeSizeInBytes * 8)) - 1;
    if(iCharCode > maxCharCode || 0 == iCharCode)
    {
        oInfo.glyphDataAdressInBits = 0;
        oInfo.advance = GetAdvance(0);
        return;
    }
    u8 inCharCodeBuffer[4];
    u8* const inCharCodeEnd = inCharCodeBuffer + SG_ARRAYSIZE(inCharCodeBuffer);
    u8* const inCharCodeBegin = inCharCodeEnd - charCodeSizeInBytes;
    SG_ASSERT(ptrdiff_t(inCharCodeBegin - inCharCodeBuffer) >= 0);
    {
        u32 tmp = iCharCode;
        u8* inCharCodeCursor = inCharCodeEnd;
        do
        {
            --inCharCodeCursor;
            *inCharCodeCursor = tmp & 0xFF;
            tmp >>= 1;
        } while(inCharCodeCursor != inCharCodeBegin);
    }
    u8 const* const inCharCodeAsu8 = inCharCodeBegin;
    size_t b = 0;
    size_t e = glyphCount;
    do
    {
        SG_ASSERT(b < e);
        size_t const m = (b + e + 1) >> 1;
        SG_ASSERT(b < m);
        SG_ASSERT(m < e);
        int const r = CompareCharCode(inCharCodeAsu8, charCodesAsu8 + m * charCodeSizeInBytes, charCodeSizeInBytes);
        if(0 == r)
        {
            oInfo.glyphDataAdressInBits = m * glyphSize.x() * glyphSize.y();
            oInfo.advance = GetAdvance(m);
            return;
        }
        else if(r > 0)
        {
            b = m;
        }
        else
        {
            e = m;
        }
    } while(b+1 != e);
    oInfo.glyphDataAdressInBits = 0;
    oInfo.advance = GetAdvance(0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int BitMapFont::GetKerning(u32 iCharCode1, u32 iCharCode2) const
{
    if(0 == kerningCount) return 0;
    SG_ASSERT(0 != kerningCount);
    u8 const* const charCodesAsu8 = reinterpret_cast<u8 const*>(kerningCharCodes);
    if(0 == charCodeSizeInBytes) { return 0; }
    u32 const maxCharCode = (1 << (charCodeSizeInBytes * 8)) - 1;
    if(iCharCode1 > maxCharCode || 0 == iCharCode1) { return 0; }
    if(iCharCode2 > maxCharCode || 0 == iCharCode2) { return 0; }
    u8 inCharCodesBuffer[8];
    u8* inCharCodesCursor = inCharCodesBuffer + SG_ARRAYSIZE(inCharCodesBuffer);
    for_range(size_t, i, 0, charCodeSizeInBytes)
    {
        --inCharCodesCursor;
        *inCharCodesCursor = (iCharCode2 >> (i * 8)) & 0xFF;
    }
    for_range(size_t, i, 0, charCodeSizeInBytes)
    {
        --inCharCodesCursor;
        *inCharCodesCursor = (iCharCode1 >> (i * 8)) & 0xFF;
    }
    SG_ASSERT(ptrdiff_t(inCharCodesCursor - inCharCodesBuffer) >= 0);
    SG_ASSERT(size_t(inCharCodesCursor - inCharCodesBuffer) == SG_ARRAYSIZE(inCharCodesBuffer) - 2 * charCodeSizeInBytes);
    u8 const* inCharCodesAsu8 = inCharCodesCursor;

    size_t b = size_t(-(int)kerningCount)-1;
    size_t e = kerningCount;
    SG_ASSERT(((b + e + 1) >> 1) == 0);
    do
    {
        SG_ASSERT(b != e);
        size_t const m = (b + e + 1) >> 1;
        SG_ASSERT(m != b);
        SG_ASSERT(m != e);
        int const r = CompareCharCode(inCharCodesAsu8, charCodesAsu8 + m * 2 * charCodeSizeInBytes, 2 * charCodeSizeInBytes);
        if(0 == r) { return GetKerning(m); }
        else if(r > 0)
            b = m;
        else
            e = m;
    } while(b+1 < e);
    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t BitMapFont::GetAdvance(size_t iIndex) const
{
    SG_ASSERT(advanceBitCount == 0 || advanceBitCount == 1 || advanceBitCount == 2 || advanceBitCount == 4);
    if(0 == advanceBitCount)
        return advancePalette[0];
    else
    {
        size_t const address = iIndex * advanceBitCount;
        size_t const adressShift = 6;
        SG_ASSERT((address >> adressShift) < advancesDataSize);
        size_t const addressMask = (1 << adressShift) - 1;
        size_t const mask = (1 << advanceBitCount) - 1;
        size_t const advanceIdx = (advancesData[address >> 6] >> (address & addressMask)) & mask;
        return advancePalette[advanceIdx];
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
int BitMapFont::GetKerning(size_t iIndex) const
{
    SG_ASSERT(kerningBitCount == 0 || kerningBitCount == 1 || kerningBitCount == 2);
    if(0 == kerningBitCount)
        return kerningPalette[0];
    else
    {
        size_t const address = iIndex * kerningBitCount;
        size_t const adressShift = 6;
        SG_ASSERT((address >> adressShift) < advancesDataSize);
        size_t const addressMask = (1 << adressShift) - 1;
        size_t const mask = (1 << kerningBitCount) - 1;
        size_t const kerningIdx = (kerningData[address >> 6] >> (address & addressMask)) & mask;
        return kerningPalette[kerningIdx];
    }
}
//=============================================================================
}
}

