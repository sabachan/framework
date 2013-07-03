#ifndef UserInterface_TypefaceFromBitMapFont_H
#define UserInterface_TypefaceFromBitMapFont_H

#include <Rendering/RenderStateDico.h>
#include <Rendering/Shader.h>
#include "Typeface.h"


namespace sg {
namespace rendering {
    class RenderDevice;
    class TextureFromOwnMemory;
}
namespace image {
    struct BitMapFont;
}
}

namespace sg {
namespace ui {
//=============================================================================
struct TextStyle;
//=============================================================================
class FontFromBitMapFont : public IFont
                         , public RefCountable
{
public:
    FontFromBitMapFont(rendering::RenderDevice const* iRenderDevice, image::BitMapFont const& iFont);
    ~FontFromBitMapFont();

    virtual float GetAdvance(u32 iCharCode, TextStyle const& iTextStyle) const override;
    virtual float GetKerning(u32 iCharCode1, u32 iCharCode2, TextStyle const& iTextStyle) const override;
    virtual void GetGlyphInfo(GlyphInfo& oInfo, u32 iCharCode, TextStyle const& iTextStyle) const override;
    virtual void GetFontInfo(FontInfo& oInfo, TextStyle const& iTextStyle) const override;
private:
    image::BitMapFont const& m_font;
    // TODO: instead of creating Material from shaders and co, it could be
    // created from another Material, by adding the texture in the database
    rendering::ShaderInputLayoutProxy m_inputLayout;
    rendering::VertexShaderProxy m_vertexShader;
    rendering::PixelShaderProxy m_pixelShader;
    rendering::PixelShaderProxy m_pixelShader_Stroke;
    rendering::RenderStateName m_blendMode;
    refptr<rendering::Material> m_material; // one per texture
    refptr<rendering::Material> m_material_Stroke;
    refptr<rendering::TextureFromOwnMemory> m_texture;
    u32 m_nx;
};
//=============================================================================
class TypefaceFromBitMapFont : public ITypeface
{
public:
    TypefaceFromBitMapFont(rendering::RenderDevice const* iRenderDevice);
    ~TypefaceFromBitMapFont();
    virtual IFont const* GetFont(TextStyle const& iTextStyle) const override;
    virtual bool GridAligned() const override;
private:
    struct Descriptor
    {
        FastSymbol familyName;
        image::BitMapFont const* font;
    public:
        //Descriptor() : familyName(), font(nullptr) {}
        Descriptor(FastSymbol const& iFamilyName, image::BitMapFont const* iFont)
            : familyName(iFamilyName)
            , font(iFont)
        { }
    };
    std::vector<Descriptor> m_descriptors;
    mutable std::vector<refptr<FontFromBitMapFont> > m_fonts;
    safeptr<rendering::RenderDevice const> m_renderDevice;
};
//=============================================================================
}
}

#endif
