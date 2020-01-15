#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "Label.h"

#include "Common.h"
#include <UserInterface/Container.h>


namespace sg {
namespace toolsui {
//=============================================================================
Label::Label(std::wstring const& iText, ui::Length const& iLineWidth)
    : m_frameProperty()
    , m_paragraphStyle()
    , m_text()
    , m_textPos()
    , m_lineWidth(iLineWidth)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    m_paragraphStyle = styleGuide.GetParagraphStyle(common.Default);
    SG_ASSERT(-1 == m_paragraphStyle.lineWidth);
    m_text.SetStyles(styleGuide.GetTypeface(common.Default), &styleGuide.GetTextStyle(common.Default), styleGuide.GetTFS(common.Default), &m_paragraphStyle);
    m_text.SetText(iText);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Label::~Label()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Label::SetText(std::wstring const& iText)
{
    m_text.SetText(iText);
    InvalidatePlacement();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Label::SetLineWidth(ui::Length const& iLineWidth)
{
    m_lineWidth = iLineWidth;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Label::VirtualResetOffset()
{
    m_frameProperty.offset.x().unit = 0;
    m_frameProperty.offset.y().unit = 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Label::VirtualAddOffset(float2 const& iOffset)
{
    m_frameProperty.offset.x().unit += iOffset.x();
    m_frameProperty.offset.y().unit += iOffset.y();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Label::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& placementBox = PlacementBox();
    bool const mustDraw = IntersectStrict(iContext.BoundingClippingBox(), placementBox);
    if(mustDraw)
    {
#if 0 // debug draw box
        ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
        drawer->DrawQuad(iContext, placementBox, Color4f(1,1,0.5f,1)*0.1f);
#endif
        m_text.Draw(iContext, m_textPos);
    }
    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Label::VirtualUpdatePlacement()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();
    float2 const textMargin = styleGuide.GetVector(common.TextMargin).Resolve(magnification, parentBox.Delta());

    SG_ASSERT_MSG(0.f == m_frameProperty.offset.x().unit || 0.f == m_lineWidth.relative, "A Label in an horizontal list must have a non-relative line width");

    float const minParentInducedLineWidth = 0.f;
    float const lineWidth = m_lineWidth.Resolve(magnification, std::max(minParentInducedLineWidth, parentBox.Delta().x() - 2.f * textMargin.x()));
    if(!EqualsWithTolerance(lineWidth, m_paragraphStyle.lineWidth, 0.1f))
    {
        m_paragraphStyle.lineWidth = lineWidth;
        m_text.SetParagraphStyle(&m_paragraphStyle);
        box2f const textBox = m_text.Box();
        float const textWidth = textBox.Delta().x();
        if(textWidth > lineWidth)
        {
            // this is for all text to use the longest line width.
            m_paragraphStyle.lineWidth = textWidth;
            m_text.SetParagraphStyle(&m_paragraphStyle);
        }
    }
    box2f const textBox = m_text.Box();
    float const contentWidth = std::max(m_paragraphStyle.lineWidth, textBox.Delta().x());
    float2 const contentInducedSize = float2(contentWidth, textBox.Delta().y()) + 2 * textMargin;
    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentInducedSize);
    SetPlacementBox(frame);
    ui::TextBoxAlignment textAlignment;
    textAlignment.alignementToAnchor = float2(0.5f,0.5f);
    textAlignment.useBaselinesInY = 0;
    textAlignment.useTextAlignmentInX = 0;
    m_textPos = frame.Center() + m_text.ComputeOffset(textAlignment);
}
//=============================================================================
}
}
#endif
