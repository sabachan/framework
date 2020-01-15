#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "Button.h"

#include "Label.h"
#include <Core/Tool.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/UniformDrawer.h>


namespace sg {
namespace toolsui {
//=============================================================================
Button::Button()
    : Button(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToContentOnly))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Button::Button(ui::FitMode2 iFitMode)
    : m_frameProperty()
    , m_sensitiveArea()
    , m_boxArea()
    , m_subContainer()
    , m_content()
    , m_fitMode(iFitMode)
    , m_userData(0)
{
    m_subContainer = new ui::SubContainer();
    RequestAddToBack(m_subContainer.get());
    m_frameProperty.size = ui::Relative(float2(1, 1));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Button::~Button()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::SetContent(ui::IMovable* iContent)
{
    if(iContent == m_content)
        return;
    if(nullptr != m_content)
        m_subContainer->RequestRemove(m_content->AsComponent());
    if(nullptr != iContent)
        m_subContainer->RequestAddToFront(iContent->AsComponent());
    m_content = iContent;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void Button::CheckConstraintsOnContent(ui::IMovable* iContent)
{
#if 0
    // If FitMode == FitToFrame, there is no constraint on item.
    // Else, at a given date, an Item must verify following constraint:
    // - There exists a width W (the minimum width of the item) such that,
    //   if the parent width is less than W, the item width is W and, if
    //   the parent width is more than W, the item width is greater or
    //   equal to W and less or equal to its parent width.
    // - After offset has been reset, If parent width is greater than the
    //   item minimum width, then the item is fully contained by the parent
    //   along width direction.
    // Note that, as these constraints must be verified "at a given date",
    // the width of the item can depends on the date or any other parameter
    // that does not depend on UI placement.

    ui::Component* c = iItem->AsComponent();
    // In the case the iTem is transfered from one parent to another, it
    // may be impossible to check it, as it cannot be added to the test
    // container.
    if(nullptr != c->Parent())
        return;

    ui::RootContainer testContainer;
    testContainer.RequestAddToFront(c);
    SG_ASSERT(c->Parent() == &testContainer);
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
    box2f const box0 = c->PlacementBox();
    float const widthAt0 = box0.Delta()[widthDirection];
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(0.5f * widthAt0)));
    box2f const box1 = c->PlacementBox();
    SG_ASSERT_MSG(widthAt0 == box1.Delta()[widthDirection],
                "The minimum width of the item must be constant.");
    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(widthAt0)));
    box2f const box2 = c->PlacementBox();
    SG_ASSERT_MSG(widthAt0 == box2.Delta()[widthDirection],
                "The minimum width of the item must be constant.");
    SG_ASSERT_MSG(0 == box2.Min()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");
    SG_ASSERT_MSG(widthAt0 == box2.Max()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");

    testContainer.SetPlacementBox(box2f::FromMinMax(float2(0), float2(2 * widthAt0)));
    box2f const box3 = c->PlacementBox();
    SG_ASSERT_MSG(widthAt0 <= box3.Delta()[widthDirection],
                "The width of the item must be greater than its minimum width (defined when its parent has width 0).");
    SG_ASSERT_MSG(2*widthAt0 >= box3.Delta()[widthDirection],
                "The width of the item must be less than its parent width when this width is greater than the item minimum width.");
    SG_ASSERT_MSG(0 <= box3.Min()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");
    SG_ASSERT_MSG(2*widthAt0 >= box3.Max()[widthDirection],
                "The item must be fully contained by the parent along width direction when parent is greater than minimum width.");

    testContainer.RequestRemove(c);
    SG_ASSERT(nullptr == c->Parent());
#endif
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::Activate()
{
    RequestFocusIFN();
    MoveToFrontOfAllUI();
    NotifyObservers();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::VirtualResetOffset()
{
    m_frameProperty.offset = ui::Unit(float2(0));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::VirtualAddOffset(float2 const& iOffset)
{
    m_frameProperty.offset += ui::Unit(iOffset);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        Activate();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& placementBox = PlacementBox();

    ButtonLikeRenderParam renderParam;
    GetButtonLikeRenderParam(renderParam, placementBox, true, IsHover(), IsClicked(), HasTerminalFocus());

    float2 const outDelta = renderParam.outBox.Delta();
    float2 const inDelta = renderParam.inBox.Delta();
    if(AllGreaterStrict(inDelta, float2(0.f)))
        drawer->DrawQuad(iContext, renderParam.inBox, renderParam.fillColor);
    if(outDelta != inDelta && AllGreaterStrict(outDelta, float2(0.f)))
        drawer->DrawFrame(iContext, renderParam.inBox, renderParam.outBox, renderParam.lineColor);

    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Button::VirtualUpdatePlacement()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();

    float2 const margins = styleGuide.GetVector(common.LineMargin1).Resolve(magnification, parentBox.Delta());

    float2 const contentSize = [&]() {
        if(m_fitMode.DoesNeedContentSize())
        {
            box2f const preFrame = m_frameProperty.Resolve(magnification, parentBox, float2(0), m_fitMode);
            box2f const preSubFrame = box2f::FromMinMax(preFrame.Min() + margins, preFrame.Max() - margins);
            m_content->ResetOffset();
            SetPlacementBox(preFrame);
            m_subContainer->SetPlacementBox(preSubFrame);
            box2f const contentBox = m_content->AsComponent()->PlacementBox();
            return contentBox.Delta() + 2 * margins;
        }
        return float2(0);
    } ();

    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentSize, m_fitMode);
    box2f const subframe = box2f::FromMinMax(frame.Min() + margins, frame.Max() - margins);
    SetPlacementBox(frame);
    m_boxArea.SetBox(frame);
    m_subContainer->SetPlacementBox(subframe);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Button::VirtualMoveFocusReturnHasMoved(ui::FocusDirection iDirection)
{
    if(!HasFocus())
    {
        RequestTerminalFocusIFN();
        return true;
    }
    switch(iDirection)
    {
    case ui::FocusDirection::None:
        SG_ASSERT_NOT_REACHED();
        break;
    case ui::FocusDirection::Left:
    case ui::FocusDirection::Right:
    case ui::FocusDirection::Up:
    case ui::FocusDirection::Down:
        return false;
    case ui::FocusDirection::MoveIn:
        return true;
    case ui::FocusDirection::MoveOut:
        return false;
    case ui::FocusDirection::Activate:
        Activate();
        return true;
    case ui::FocusDirection::Validate:
        Activate();
        return true;
    case ui::FocusDirection::Cancel:
    case ui::FocusDirection::Next:
    case ui::FocusDirection::Previous:
        return false;
    default:
        SG_ASSERT_NOT_REACHED();
        return false;
    }
    return false;
}
//=============================================================================
TextButton::TextButton(std::wstring const& iText)
{
    m_label = new Label(iText);
    this->SetContent(m_label.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextButton::~TextButton()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextButton::SetText(std::wstring const& iText)
{
    m_label->SetText(iText);
}
//=============================================================================
}
}

#endif
