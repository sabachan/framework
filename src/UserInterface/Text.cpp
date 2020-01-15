#include "stdafx.h"

#include "Text.h"

#include "TextFormatScript.h"
#include "TextModifier.h"
#include "TextRenderer.h"
#include "Typeface.h"
#include <Core/VectorOfScopedPtr.h>
#include <Rendering/Material.h>

namespace sg {
namespace ui {
//=============================================================================
float2 ComputeTextBoxOffset(TextRenderer::Cache const& iTextCache, TextBoxAlignment const& iAlignment)
{
    box2f box = iTextCache.BoundingBox();
    SG_ASSERT(0 <= iAlignment.useBaselinesInY && iAlignment.useBaselinesInY <= 1.f);
    if(iAlignment.useBaselinesInY > 0)
    {
        box.min.y() = lerp(box.min.y(), 0.f, iAlignment.useBaselinesInY);
        box.max.y() = lerp(box.max.y(), iTextCache.LastBaseline(), iAlignment.useBaselinesInY);
    }
    float2 offset;
    offset = -lerp(box.Min(), box.Max(), iAlignment.alignementToAnchor);
    SG_ASSERT(0 <= iAlignment.useTextAlignmentInX && iAlignment.useTextAlignmentInX <= 1.f);
    if(iAlignment.useTextAlignmentInX > 0)
        offset.x() *= (1.f-iAlignment.useTextAlignmentInX);
    return offset;
}
//=============================================================================
Text::Text()
    : m_text()
    , m_tfs(nullptr)
    , m_textStyle(nullptr)
    , m_paragraphStyle(nullptr)
    , m_typeface(nullptr)
    , m_textCache()
    , m_isUpToDate(false)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Text::Text(lstring const& iText, ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle)
    : Text()
{
    SetStyles(iTypeface, iTextStyle, iParagraphStyle);
    SetText(iText);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Text::Text(lstring const& iText, ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
    : Text()
{
    SetStyles(iTypeface, iTFS, iParagraphStyle);
    SetText(iText);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Text::Text(lstring const& iText, ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
    : Text()
{
    SetStyles(iTypeface, iTextStyle, iTFS, iParagraphStyle);
    SetText(iText);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::SetParagraphStyle(ParagraphStyle const* iParagraphStyle)
{
    SG_ASSERT(nullptr != iParagraphStyle);
    m_paragraphStyle = iParagraphStyle;
    m_isUpToDate = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::SetStyles_Impl(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
{
    SG_ASSERT(nullptr != iTypeface);
    SG_ASSERT(nullptr != iTextStyle);
    //SG_ASSERT(nullptr != iTFS);
    SG_ASSERT(nullptr != iParagraphStyle);
    m_tfs = iTFS;
    m_textStyle = iTextStyle;
    m_paragraphStyle = iParagraphStyle;
    m_typeface = iTypeface;
    m_isUpToDate = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, ParagraphStyle const* iParagraphStyle)
{
    SetStyles_Impl(iTypeface, iTextStyle, nullptr, iParagraphStyle);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::SetStyles(ITypeface const* iTypeface, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
{
    TFSInstruction const* defaultInst = iTFS->GetInstructionIFP("default");
    SG_ASSERT_MSG(nullptr != defaultInst, "A default style is expected in this Text Format Script.");
    SG_ASSERT_MSG(TFSInstruction::Type::Style == defaultInst->GetType(), "In any Text Format Script, default should be a style.");
    TFSStyle const* defaultStyleInst = checked_cast<TFSStyle const*>(defaultInst);
    TextStyle const* textStyle = defaultStyleInst->GetStyle();
    SG_ASSERT(nullptr != textStyle);
    SetStyles_Impl(iTypeface, textStyle, iTFS, iParagraphStyle);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::SetStyles(ITypeface const* iTypeface, TextStyle const* iTextStyle, TextFormatScript const* iTFS, ParagraphStyle const* iParagraphStyle)
{
    SetStyles_Impl(iTypeface, iTextStyle, iTFS, iParagraphStyle);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::SetText(lstring const& iText)
{
    m_text = iText;
    m_isUpToDate = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f Text::Box()
{
    PrepareIFN();
    return m_textCache.BoundingBox();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::Draw(ui::DrawContext const& iContext, float2 const& iPosition, size_t iLayer)
{
    PrepareIFN();
    ui::TextRenderer::Render(iPosition,
                             m_textCache,
                             iContext,
                             iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::Draw(ui::DrawContext const& iContext, float2 const& iPosition, TextBoxAlignment const& iAlignment, size_t iLayer)
{
    float2 const offset = ComputeOffset(iAlignment);
    Draw(iContext, iPosition + offset, iLayer);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float2 Text::ComputeOffset(TextBoxAlignment const& iAlignment)
{
    PrepareIFN();
    return ComputeTextBoxOffset(m_textCache, iAlignment);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Text::GetGlyphCount()
{
    PrepareIFN();
    return TextRenderer::GetGlyphCount(m_positionCache);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
box2f Text::GetGlyphBox(size_t i)
{
    PrepareIFN();
    return TextRenderer::GetGlyphBox(m_positionCache, i);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
float2 Text::GetPenPosition(size_t i)
{
    PrepareIFN();
    return TextRenderer::GetPenPosition(m_positionCache, i);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Text::GetNearestGlyph(float2 const& iPosition)
{
    PrepareIFN();
    return TextRenderer::GetNearestGlyph(m_positionCache, iPosition);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Text::GetNearestInterGlyph(float2 const& iPosition)
{
    PrepareIFN();
    return TextRenderer::GetNearestInterGlyph(m_positionCache, iPosition);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::GetSelectionBoxes(std::vector<box2f>& oBoxes, size_t b, size_t e)
{
    PrepareIFN();
    return TextRenderer::GetSelectionBoxes(m_positionCache, oBoxes, b, e);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Text::Prepare()
{
    VectorOfScopedPtr<ui::ITextModifier const> textModifiers;
    if(nullptr != m_tfs)
        m_tfs->Run(textModifiers, m_text);
    else
    {
        // TODO: Assert no script in text.
        SG_ASSERT_NOT_IMPLEMENTED();
    }
    ArrayView<ui::ITextModifier const* const> modifiers;
    modifiers = ArrayView<ui::ITextModifier const* const>(textModifiers.data(), textModifiers.size());

    SG_ASSERT(nullptr != m_typeface);
    SG_ASSERT(nullptr != m_textStyle);
    SG_ASSERT(nullptr != m_paragraphStyle);
    ui::TextRenderer::Prepare(m_positionCache,
                              m_textCache,
                              m_text,
                              *m_typeface,
                              *m_textStyle,
                              *m_paragraphStyle,
                              modifiers);
    m_isUpToDate = true;
}
//=============================================================================
}
}
