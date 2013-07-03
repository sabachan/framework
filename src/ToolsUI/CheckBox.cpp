#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "CheckBox.h"

#include "Label.h"
#include <Core/Tool.h>
#include <Image/FontSymbols.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/UniformDrawer.h>


namespace sg {
namespace toolsui {
//=============================================================================
//namespace {
//class CheckBoxCommon : public Singleton<CheckBoxCommon>
//{
//};
//}
//=============================================================================
namespace {
    wchar_t checkedBoxStr[] = { '#','s','y','m','b','o','l','s','{', image::symbols::CheckedBox, '}','\0' };
    wchar_t uncheckedBoxStr[] = { '#','s','y','m','b','o','l','s','{', image::symbols::UncheckedBox, '}','\0' };
}
//=============================================================================
CheckBox::CheckBox()
    : CheckBox(ui::FitMode2(ui::FitMode::FitToMaxContentAndFrame, ui::FitMode::FitToContentOnly))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CheckBox::CheckBox(ui::FitMode2 iFitMode)
    : m_frameProperty()
    , m_sensitiveArea()
    , m_boxArea()
    , m_list()
    , m_content()
    , m_checkBox()
    , m_fitMode(iFitMode)
    , m_state(false)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();

    ui::Length2 margins = styleGuide.GetVector(common.LineMargin1);
    ui::HorizontalListLayout::Properties listprop;
    listprop.margins.left = margins.x();
    listprop.margins.right = margins.x();
    listprop.margins.top = margins.y();
    listprop.margins.bottom = margins.y();
    listprop.margins.interItem; // = 0;
    listprop.widthFitMode = m_fitMode.y();
    m_list = new ui::HorizontalListLayout(common.GetMagnifier(), listprop);
    RequestAddToBack(m_list.get());

    m_frameProperty.size = ui::Relative(float2(1, 1));

    m_checkBox = new Label(uncheckedBoxStr, ui::Unit(-1.f));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
CheckBox::~CheckBox()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::SetState(bool iState)
{
    if(iState != m_state)
    {
        m_state = iState;
        m_checkBox->SetText(m_state ? checkedBoxStr : uncheckedBoxStr);
        NotifyObservers();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::SetContent(ui::IMovable* iContent)
{
    if(iContent == m_content)
        return;
    if(nullptr != m_content)
    {
        m_list->RemoveAllItems();
    }
    if(nullptr != iContent)
    {
        iContent->ResetOffset();
        m_list->AppendExpansibleItem(iContent->AsComponent(), ui::Unit(10), 1);
        m_list->AppendItem(m_checkBox.get());
    }
    m_content = iContent;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void CheckBox::CheckConstraintsOnContent(ui::IMovable* iContent)
{
    ui::Component* c = iContent->AsComponent();
    // In the case the iTem is transfered from one parent to another, it
    // may be impossible to check it, as it cannot be added to the test
    // container.
    if(nullptr != c->Parent())
        return;

    // TODO
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::VirtualResetOffset()
{
    m_frameProperty.offset = ui::Unit(float2(0));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::VirtualAddOffset(float2 const& iOffset)
{
    m_frameProperty.offset += ui::Unit(iOffset);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        MoveToFrontOfAllUI();
        FlipState();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& placementBox = PlacementBox();

    ButtonLikeRenderParam renderParam;
    GetButtonLikeRenderParam(renderParam, placementBox, true, IsHover(), IsClicked());

    float2 const outDelta = renderParam.outBox.Delta();
    float2 const inDelta = renderParam.inBox.Delta();
    if(AllGreaterStrict(inDelta, float2(0.f)))
        drawer->DrawQuad(iContext, renderParam.inBox, renderParam.fillColor);
    if(outDelta != inDelta && AllGreaterStrict(outDelta, float2(0.f)))
        drawer->DrawFrame(iContext, renderParam.inBox, renderParam.outBox, renderParam.lineColor);

    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CheckBox::VirtualUpdatePlacement()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();

    float2 const contentSize = [&]() {
        if(m_fitMode.DoesNeedContentSize())
        {
            box2f const preFrame = m_frameProperty.Resolve(magnification, parentBox, float2(0), m_fitMode);
            SetPlacementBox(preFrame);
            box2f const contentBox = m_list->PlacementBox();
            return contentBox.Delta();
        }
        return float2(0);
    } ();

    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, contentSize, m_fitMode);
    SetPlacementBox(frame);
    m_boxArea.SetBox(frame);
}
//=============================================================================
TextCheckBox::TextCheckBox(std::wstring const& iText)
{
    m_label = new Label(iText);
    this->SetContent(m_label.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TextCheckBox::~TextCheckBox()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void TextCheckBox::SetText(std::wstring const& iText)
{
    m_label->SetText(iText);
}
//=============================================================================
}
}

#endif
