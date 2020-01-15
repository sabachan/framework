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

    size_t const glyphCount = iFont.glyphCount;
    size_t const margin = 1;
    size_t const charW = iFont.glyphSize.x() + margin;
    size_t const charH = iFont.glyphSize.y() + margin;
    size_t w = 64;
    size_t nx = 1;
    size_t ny = glyphCount;
    size_t h = all_ones;
    while(h > 2 * w)
    {
        w *= 2;
        nx = (w-margin) / charW;
        ny = (glyphCount + nx-1) / nx;
        h = margin + ny * charH;
    }
    m_nx = nx;
    size_t2 const wh(w,h);
    m_texture = new rendering::TextureFromOwnMemory(iRenderDevice, rendering::TextureFromMemory::ColorFormat::R8, uint2(wh));
    auto img = m_texture->GetAsImageForModification<u8>();
    image::DrawRect(img, box2u::FromMinMax(uint2(0), uint2(wh)), image::brush::Fill(u8(0)));
    size_t2 pos(margin,margin);
    u32 const baseline = iFont.baseline;
    for(size_t i = 0; i < glyphCount; ++i)
    {
        if(pos.x() + charW > w)
        {
            pos.x() = margin;
            pos.y() += charH;
            SG_ASSERT(pos.y() + charH <= h);
        }
        u32 const charCode = iFont.GetGlyphCharCode(i);
        image::DrawText(img, pos + uint2(0, baseline), &charCode, 1, image::brush::Fill(u8(255)), iFont);
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
    int const margin = 1;
    int const ext = 3;
    int const baseline = m_font.baseline;
    int const interCharSpace = m_font.advanceMonospace - m_font.glyphSize.x();
    int const cw = m_font.glyphSize.x();
    int const ch = m_font.glyphSize.y();
    int const nx = int(m_nx);
    int const slotW = m_font.glyphSize.x() + margin;
    int const slotH = m_font.glyphSize.y() + margin;
    int const glyphIndex = int(m_font.GetGlyphIndex(iCharCode));
    box2i const subbox = box2i::FromMinDelta(int2(margin + (glyphIndex%nx) * slotW, margin + (glyphIndex/nx) * slotH), int2(cw, ch));
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
