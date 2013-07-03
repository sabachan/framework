#include "stdafx.h"

#include "TypefaceFromBitMapFont.h"

#include <Image/BitMapFont.h>
#include <Image/DebugImage.h>
#include <Image/Draw.h>
#include <Image/Image.h>
#include <Rendering/Material.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/TextureFromMemory.h>
#include "TextRenderer.h"
#include "TextStyle.h"

namespace sg {
namespace ui {
//=============================================================================
FontFromBitMapFont::FontFromBitMapFont(rendering::RenderDevice const* iRenderDevice, image::BitMapFont const& iFont)
    : m_font(iFont)
    , m_nx(0)
{
    refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/UserInterface/Shaders/V_UI_Text2D.hlsl"), "vmain");
    m_vertexShader = vsDesc->GetProxy();
    m_inputLayout = rendering::ShaderInputLayoutProxy(TextRenderer::VertexDescriptor(), m_vertexShader);
    refptr<rendering::PixelShaderDescriptor> psDesc = new rendering::PixelShaderDescriptor(FilePath("src:/UserInterface/Shaders/P_UI_Text2D.hlsl"), "pmain");
    m_pixelShader = psDesc->GetProxy();
    refptr<rendering::PixelShaderDescriptor> psDesc_Stroke = new rendering::PixelShaderDescriptor(FilePath("src:/UserInterface/Shaders/P_UI_Text2D_Stroke.hlsl"), "pmain");
    m_pixelShader_Stroke = psDesc_Stroke->GetProxy();
    m_blendMode = rendering::RenderStateName("Premultiplied Alpha Blending");

    u32 const margin = 1;
    u32 const charW = iFont.glyphSize.x() + margin;
    u32 const charH = iFont.glyphSize.y() + margin;
    u32 w = 64;
    u32 nx = 1;
    u32 ny = 256;
    u32 h = all_ones;
    while(h > 2 * w)
    {
        w *= 2;
        nx = (w-margin) / charW;
        ny = (256+nx-1) / nx;
        h = margin + ny * charH;
    }
    m_nx = nx;
    uint2 const wh(w,h);
    m_texture = new rendering::TextureFromOwnMemory(iRenderDevice, rendering::TextureFromMemory::ColorFormat::R8, wh);
    auto img = m_texture->GetAsImageForModification<u8>();
    image::DrawRect(img, box2u::FromMinMax(uint2(0), wh), image::brush::Fill(u8(0)));
    std::string str = " ";
    uint2 pos(margin,margin);
    u32 const baseline = iFont.baseline;
    for(size_t i = 0; i < 256; ++i)
    {
        if(pos.x() + charW > w)
        {
            pos.x() = margin;
            pos.y() += charH;
            SG_ASSERT(pos.y() + charH <= h);
        }
        str[0] = char(i);
        image::DrawText(img, pos + uint2(0, baseline), str, image::brush::Fill(u8(255)), iFont);
        pos.x() += charW;
    }
    SG_DEBUG_IMAGE_STEP_INTO(img, true);
    m_texture->Update_AssumeModified();

    std::pair<rendering::ShaderResourceName, rendering::IShaderResource const*> resources[] = {
        std::make_pair(rendering::ShaderResourceName("texture_glyph_atlas"), m_texture.get()),
    };
    //std::pair<rendering::ShaderResourceName, ID3D11SamplerState*> samplers[] = {
    //};
    m_material = new rendering::Material(m_inputLayout,
                                         m_vertexShader,
                                         m_pixelShader,
                                         m_blendMode,
                                         rendering::Material::ConstantsView(),
                                         rendering::Material::ResourcesView(resources, SG_ARRAYSIZE(resources)),
                                         rendering::Material::SamplersView(),
                                         0 );
    m_material_Stroke = new rendering::Material(m_inputLayout,
                                                m_vertexShader,
                                                m_pixelShader_Stroke,
                                                m_blendMode,
                                                rendering::Material::ConstantsView(),
                                                rendering::Material::ResourcesView(resources, SG_ARRAYSIZE(resources)),
                                                rendering::Material::SamplersView(),
                                                -1 );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
FontFromBitMapFont::~FontFromBitMapFont()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float FontFromBitMapFont::GetAdvance(u32 iCharCode, TextStyle const& iTextStyle) const
{
    SG_UNUSED(iTextStyle);
    image::BitMapFont::GlyphInfo glyphInfo;
    m_font.GetGlyphInfo(glyphInfo, iCharCode);
    return float(glyphInfo.advance);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float FontFromBitMapFont::GetKerning(u32 iCharCode1, u32 iCharCode2, TextStyle const& iTextStyle) const
{
    SG_UNUSED(iTextStyle);
    return float(m_font.GetKerning(iCharCode1, iCharCode2));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FontFromBitMapFont::GetFontInfo(FontInfo& oInfo, TextStyle const& iTextStyle) const
{
    SG_UNUSED(iTextStyle);
    float const baseline = float(m_font.baseline);
    oInfo.ascent = baseline;
    oInfo.descent = m_font.glyphSize.y() - baseline;
    oInfo.interCharSpace = float(m_font.advanceMonospace - m_font.glyphSize.x());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void FontFromBitMapFont::GetGlyphInfo(GlyphInfo& oInfo, u32 iCharCode, TextStyle const& iTextStyle) const
{
    u32 const margin = 1;
    i32 const ext = 3;
    i32 const baseline = m_font.baseline;
    i32 const interCharSpace = m_font.advanceMonospace - m_font.glyphSize.x();
    u32 const cw = m_font.glyphSize.x();
    u32 const ch = m_font.glyphSize.y();
    u32 const nx = m_nx;
    u32 const slotW = m_font.glyphSize.x() + margin;
    u32 const slotH = m_font.glyphSize.y() + margin;
    box2i const subbox = box2i::FromMinDelta(int2(margin + (iCharCode%nx) * slotW, margin + (iCharCode/nx) * slotH), int2(cw, ch));
    uint2 const wh = m_texture->ShaderResourceResolution()->Get();
    float2 const oowh = float2(1.f) / float2(wh);

    SG_ASSERT(iTextStyle.strokeSize >= 0.f);
    u8 const quantizedStrokeSize = checked_numcastable(roundi(iTextStyle.strokeSize));
    oInfo.materials[0] = m_material.get();
    if(0 != quantizedStrokeSize)
        oInfo.materials[1] = m_material_Stroke.get();
    else
        oInfo.materials[1] = nullptr;
    oInfo.renderbox = box2f::FromMinMax(float2(float(-ext), float(-baseline-ext)), float2(float(cw+ext), float(-baseline+ch+ext)));
    oInfo.uvBox = box2f::FromMinDelta(float2(subbox.min) - float2(float(ext)), float2(float(cw), float(ch)) + float2(2.f*ext)) * oowh;
    oInfo.quadVertexData1 = ubyte4(checked_numcastable(subbox.min.x()),
                                   checked_numcastable(subbox.min.y()),
                                   checked_numcastable(subbox.max.x()),
                                   checked_numcastable(subbox.max.y()));
    oInfo.quadVertexData2 = ubyte4(quantizedStrokeSize, 0,0,0);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TypefaceFromBitMapFont::TypefaceFromBitMapFont(rendering::RenderDevice const* iRenderDevice)
    : m_descriptors()
    , m_fonts()
    , m_renderDevice(iRenderDevice)
{
    ArrayView<image::BitMapFont const> fonts = image::GetAlwaysAvailableBitmapFonts();

    for_range(size_t, i, 0, fonts.size())
    {
        m_descriptors.emplace_back(fonts[i].familyName, &fonts[i]);
    }
    m_fonts.resize(m_descriptors.size());

}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TypefaceFromBitMapFont::~TypefaceFromBitMapFont()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
IFont const* TypefaceFromBitMapFont::GetFont(TextStyle const& iTextStyle) const
{
    size_t best = all_ones;
    size_t bestScore = std::numeric_limits<size_t>::max();
    for_range(size_t, i, 0, m_descriptors.size())
    {
        size_t score = 0;
        if(iTextStyle.fontFamilyName != m_descriptors[i].familyName) { if(bestScore < 64) { continue; } score += 64; }
        image::BitMapFont const& font = *m_descriptors[i].font;
        score += roundi(4 * std::abs(3 * (iTextStyle.size - font.glyphSize.y()) - 1)); // prefer small font
        if(iTextStyle.bold != (0 != (font.flags & image::BitMapFont::Flag::Bold))) { score += 1; }
        if(iTextStyle.italic != (0 != (font.flags & image::BitMapFont::Flag::Italic))) { score += 2; }

        if(score < bestScore)
        {
            best = i;
            bestScore = score;
            if(0 == score)
                break;
        }
    }
    SG_ASSERT(all_ones != best);
    SG_ASSERT(best < m_fonts.size());
    if(nullptr == m_fonts[best])
    {
        m_fonts[best] = new FontFromBitMapFont(m_renderDevice.get(), *m_descriptors[best].font);
    }
    return m_fonts[best].get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool TypefaceFromBitMapFont::GridAligned() const
{
    return true;
}
//=============================================================================
}
}
