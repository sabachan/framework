#ifndef UserInterface_Typeface_H
#define UserInterface_Typeface_H

#include <Core/SmartPtr.h>
#include <Math/Box.h>
#include <Math/Vector.h>
#include <Rendering/Material.h>

namespace sg {
namespace rendering {
    class Material;
}
}

namespace sg {
namespace ui {
//=============================================================================
struct TextStyle;
//=============================================================================
struct GlyphInfo
{
    // TODO: how to:
    // - ask to use different number of materials ?
    // - give more or less data for quad vertices ?
    safeptr<rendering::Material> materials[2];
    box2f uvBox;
    box2f renderbox;
    ubyte4 quadVertexData1;
    ubyte4 quadVertexData2;
public:
    GlyphInfo() {}
    GlyphInfo(uninitialized_t)
        : uvBox(uninitialized)
        , renderbox(uninitialized)
        , quadVertexData1(uninitialized)
        , quadVertexData2(uninitialized)
    {}
};
//=============================================================================
struct FontInfo
{
    float ascent;
    float descent;
    float interCharSpace; // This space is considered to be after the character
                          // (advance = char width + inter char space).
};
//=============================================================================
class IFont : public SafeCountable
{
public:
    virtual ~IFont() {}
    virtual float GetAdvance(u32 iCharCode, TextStyle const& iTextStyle) const = 0;
    virtual float GetKerning(u32 iCharCode1, u32 iCharCode2, TextStyle const& iTextStyle) const = 0;

    virtual void GetGlyphInfo(GlyphInfo& oInfo, u32 iCharCode, TextStyle const& iTextStyle) const = 0;
    virtual void GetFontInfo(FontInfo& oInfo, TextStyle const& iTextStyle) const = 0;
};
//=============================================================================
class ITypeface : public RefAndSafeCountable
{
public:
    virtual ~ITypeface() {}
    virtual IFont const* GetFont(TextStyle const& iTextStyle) const = 0;
    virtual bool GridAligned() const = 0;
};
//=============================================================================
}
}

#endif
