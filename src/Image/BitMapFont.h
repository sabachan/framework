#ifndef Image_BitMapFont_H
#define Image_BitMapFont_H

#include <Core/ArrayView.h>
#include <Math/Vector.h>
#include <Math/NumericalUtils.h>

namespace sg {
namespace image {
//=============================================================================
// Lightweight bit map font useful for debug drawing
// - can be fully declared at compile time
// - generated by FontCodeGenerator from object script file
struct BitMapFont
{
    SG_NON_COPYABLE(BitMapFont)
    BitMapFont() = delete;
public:
    struct Flag
    {
        enum : u8
        {
            Bold   = 0x01,
            Italic = 0x02,
        };
    };
public:
    char const* familyName;
    size_t const glyphCount;
    size_t const kerningCount;
    ubyte2 const glyphSize;
    u8 const flags;
    u8 const advanceMonospace;
    u8 const baseline;
    u8 const charCodeSizeInBytes;
    u8 const advanceBitCount;
    u8 const kerningBitCount;
    u8 const advancePalette[16];
    i8 const kerningPalette[4];
    u8 const* const charCodes;
    u64 const* const glyphsData;
    u64 const* const advancesData;
    u8 const* const kerningCharCodes;
    u64 const* const kerningData;
    SG_CODE_FOR_ASSERT(size_t const charCodesSize);
    SG_CODE_FOR_ASSERT(size_t const glyphsDataSize);
    SG_CODE_FOR_ASSERT(size_t const advancesDataSize);
    SG_CODE_FOR_ASSERT(size_t const kerningCharCodesSize);
    SG_CODE_FOR_ASSERT(size_t const kerningDataSize);
public:
    struct GlyphInfo
    {
        size_t glyphDataAdressInBits;
        size_t advance;
    };
    void GetGlyphInfo(GlyphInfo& oInfo, u32 iCharCode) const;
    int GetKerning(u32 iCharCode1, u32 iCharCode2) const;
private:
    size_t GetAdvance(size_t iIndex) const;
    int GetKerning(size_t iIndex) const;
};
//=============================================================================
ArrayView<BitMapFont const> GetAlwaysAvailableBitmapFonts();
//=============================================================================
}
}
#endif
