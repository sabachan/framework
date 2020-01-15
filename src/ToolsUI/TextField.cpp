#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "TextField.h"

#include "Common.h"
#include <UserInterface/Container.h>
#include <System/Clipboard.h>
#include <System/KeyboardUtils.h>
#include <Core/StringFormat.h>
#include <cwchar>
#include <locale>
#include <sstream>

namespace sg {
namespace toolsui {
//=============================================================================
TextField::TextField(std::wstring const& iText, ui::Length const& iLineWidth)
    : EditableText(Common::Get().GetMagnifier())
    , m_cursorVisibilityStabilizer(1.f)
    , m_cursorBlinking(1.f)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::ParagraphStyle const& paragraphStyle = styleGuide.GetParagraphStyle(common.Default);
    SG_ASSERT(-1 == paragraphStyle.lineWidth);
    SetStyles(styleGuide.GetTypeface(common.Default), &styleGuide.GetTextStyle(common.Default), styleGuide.GetTFS(common.Default), &paragraphStyle);
    SetLineWidth(iLineWidth);
    SetText(iText);
    m_cursorBlinking.Play();
    m_cursorVisibilityStabilizer.Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextField::~TextField()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextField::SetLineWidth(ui::Length const& iLineWidth)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::Length2 const textMargin = styleGuide.GetVector(common.TextMargin);
    Properties p;
    p.lineWidth = iLineWidth;
    p.overrideParagraphLineWidth = true;
    p.margins = ui::LengthBox2::FromMinMax(-textMargin, textMargin);
    SetProperties(p);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextField::VirtualOnRefreshCursor()
{
    m_cursorVisibilityStabilizer.SetValue(0.f);
    m_cursorVisibilityStabilizer.Play();
    m_cursorBlinking.SetValue(0.f);
    m_cursorBlinking.Play();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextField::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& placementBox = PlacementBox();
    bool const mustDraw = IntersectStrict(iContext.BoundingClippingBox(), placementBox);
    if(mustDraw)
    {
        ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);

        bool const isHover = GetSensitiveArea().IsPointerInside();
        bool const isInEditionMode = HasFocus() && IsInEditionMode();
        bool const hasPreFocus = HasFocus() && !IsInEditionMode();

        ButtonLikeRenderParam renderParam;
        GetButtonLikeRenderParam(renderParam, placementBox, false, isHover, false, hasPreFocus);

        float2 const textPos = TextPosition();
        if(isHover || hasPreFocus)
        {
            float2 const outDelta = renderParam.outBox.Delta();
            float2 const inDelta = renderParam.inBox.Delta();
            if(outDelta != inDelta && AllGreaterStrict(outDelta, float2(0.f)))
                drawer->DrawFrame(iContext, renderParam.inBox, renderParam.outBox, renderParam.lineColor);
        }
        if(isInEditionMode)
        {
            ArrayView<box2f const> boxes = SelectionBoxes();
            for(box2f const& box: boxes)
            {
                if(box.NVolume_NegativeIfNonConvex() > 0.f)
                    drawer->DrawQuad(iContext, textPos + box, renderParam.fillColor2);
            }
        }
        DrawableText().Draw(iContext, textPos);
        if(isInEditionMode)
        {
            float const tb = m_cursorBlinking.GetValue();
            float const bf = tweening::unitCubicStep(tweening::unitTriangle(tb));
            float const tv = m_cursorVisibilityStabilizer.GetValue();
            float const vf = inverselerp(1.f, 0.9f, tv);
            float const t = std::max(bf, vf);
            box2f const box = textPos + CursorBox();
            drawer->DrawQuad(iContext, box, t * Color4f(1,1,1,1));
        }
    }

    parent_type::SkipVirtualOnDraw(iContext);
}
//=============================================================================
}
}
#endif
