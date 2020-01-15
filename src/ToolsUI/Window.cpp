#include "stdafx.h"

#include <Core/Config.h>
#if SG_ENABLE_TOOLS

#include "Window.h"

#include "Common.h"
#include "Label.h"
#include <Core/Observer.h>
#include <Image/FontSymbols.h>
#include <UserInterface/FramingContainer.h>
#include <UserInterface/GenericStyleGuide.h>
#include <UserInterface/UniformDrawer.h>

namespace sg {
namespace toolsui {
//=============================================================================
template<bool IsModal>
WindowHandlerImpl<IsModal>::WindowHandlerImpl()
    : m_window()
    , m_shown(false)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
WindowHandlerImpl<IsModal>::WindowHandlerImpl(Window* iWindow)
    : WindowHandlerImpl()
{
    SetWindow_AssumeNone(iWindow);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
WindowHandlerImpl<IsModal>::~WindowHandlerImpl()
{
    Show(false);
    ReleaseWindow_AssumeNotShown();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
void WindowHandlerImpl<IsModal>::Show(bool iValue)
{
    if(iValue != IsShown())
    {
        SG_ASSERT(nullptr != m_window);
        if(iValue)
        {
            Common const& common = Common::Get();
            if(IsModal)
                common.ModalContainer()->RequestAddToFront(m_window.get());
            else
                common.WindowContainer()->RequestAddToFront(m_window.get());
            m_shown = true;
        }
        else
        {
            SG_ASSERT(!IsModal || m_window->Parent() == Common::Get().ModalContainer());
            SG_ASSERT(IsModal || m_window->Parent() == Common::Get().WindowContainer());
            m_window->Parent()->RequestRemove(m_window.get());
            m_shown = false;
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
bool WindowHandlerImpl<IsModal>::IsShown()
{
    SG_ASSERT(nullptr != m_window || !m_shown);
    return m_shown;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
void WindowHandlerImpl<IsModal>::ReleaseWindow_AssumeNotShown()
{
    SG_ASSERT(!IsShown());
    if(nullptr != m_window)
        m_window->GetCloseEvent().UnregisterObserver(this);
    m_window = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
void WindowHandlerImpl<IsModal>::SetWindow_AssumeNone(Window* iWindow)
{
    SG_ASSERT(nullptr == m_window);
    SG_ASSERT(!IsShown());
    m_window = iWindow;
    m_window->GetCloseEvent().RegisterObserver(this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool IsModal>
void  WindowHandlerImpl<IsModal>::VirtualOnNotified(WindowCloseEvent const* iCloseEvent)
{
    SG_ASSERT(nullptr != m_window);
    SG_ASSERT(iCloseEvent == &m_window->GetCloseEvent());
    m_shown = false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template class WindowHandlerImpl<false>;
template class WindowHandlerImpl<true>;
//=============================================================================
class WindowButton : public ui::Container
                   , public ui::IMovable
                   , public ActivableMessage
                   , private ui::ISensitiveAreaListener
{
    typedef ui::Container parent_type;
    PARENT_SAFE_COUNTABLE(ui::Container);
public:
    enum class Type
    {
        Close,
        Info,
        MinimizeWindow,
        MaximizeWidow,
        RestoreWindow,
    };
    WindowButton(Type iType);
    ~WindowButton();
    virtual void VirtualResetOffset() override { m_frameProperty.offset = ui::Unit(float2(0)); }
    virtual void VirtualAddOffset(float2 const& iOffset) override { m_frameProperty.offset += ui::Unit(iOffset); }
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override
    {
        parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
        m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, m_boxArea, *this);
    }
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
private:
    bool IsHover() const { return m_sensitiveArea.IsPointerInsideFree(); }
    bool IsClicked() const { return m_sensitiveArea.IsClickedValidable(0); }
private:
    ui::FrameProperty m_frameProperty;
    ui::SensitiveArea m_sensitiveArea;
    ui::BoxArea m_boxArea;
    ui::TextStyle m_textStyle;
    ui::ParagraphStyle m_paragraphStyle;
    ui::Text m_text;
    float2 m_textPos;
    Type m_type;
};
//=============================================================================
WindowButton::WindowButton(Type iType)
    : ActivableMessage(Message(static_cast<u32>(iType)))
    , m_frameProperty()
    , m_sensitiveArea()
    , m_boxArea()
    , m_textStyle()
    , m_paragraphStyle()
    , m_text()
    , m_textPos()
    , m_type(iType)
{
    wchar_t Close[] = { image::symbols::ObliqueCross , L'\0' };
    wchar_t MinimizeWindow[] = { image::symbols::Minimized , L'\0' };
    wchar_t MaximizeWidow[] = { image::symbols::Maximized , L'\0' };
    wchar_t RestoreWindow[] = { image::symbols::Windowed , L'\0' };
    wchar_t Info[] = { image::symbols::Info , L'\0' };
    switch(iType)
    {
    case Type::Close: m_text.SetText(Close); break;
    case Type::MinimizeWindow: m_text.SetText(MinimizeWindow); break;
    case Type::MaximizeWidow: m_text.SetText(MaximizeWidow); break;
    case Type::RestoreWindow: m_text.SetText(RestoreWindow); break;
    case Type::Info: m_text.SetText(Info); break;
    default: SG_ASSUME_NOT_REACHED();
    }
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    m_paragraphStyle = styleGuide.GetParagraphStyle(common.Default);
    m_textStyle = styleGuide.GetTextStyle(common.Default);
    m_textStyle.fontFamilyName = FastSymbol("tech symbols");
    SG_ASSERT(-1 == m_paragraphStyle.lineWidth);
    m_text.SetStyles(styleGuide.GetTypeface(common.Default), &m_textStyle, styleGuide.GetTFS(common.Default), &m_paragraphStyle);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
WindowButton::~WindowButton()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowButton::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        MoveToFrontOfAllUI();
        NotifyObservers();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowButton::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();

    box2f const& placementBox = PlacementBox();
    box2f const box = placementBox;
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);

    float const lineThickness = styleGuide.GetLength(IsClicked() ? common.LineThickness1 : common.LineThickness0).Resolve(magnification, std::numeric_limits<float>::lowest());
    if(AllGreaterStrict(box.Delta(), float2(2.f * lineThickness)))
    {
        box2f const inBox = box2f::FromMinMax(box.Min() + lineThickness, box.Max() - lineThickness);
        //Color4f const color = styleGuide.GetLinearColor(IsHover() ? common.FillColorB1 : common.FillColorB);
        Color4f const color = styleGuide.GetLinearColor(common.FillColorA2);
        if(IsHover())
            drawer->DrawQuad(iContext, box, color);
        //Color4f const lineColor = styleGuide.GetLinearColor(common.LineColorB);
        //drawer->DrawFrame(iContext, inBox, box, lineColor);
    }
    m_text.Draw(iContext, m_textPos);
    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WindowButton::VirtualUpdatePlacement()
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();
    float2 const buttonSize = styleGuide.GetVector("WindowButtonSize").Resolve(magnification, float2(std::numeric_limits<float>::lowest()));
    box2f const frame = m_frameProperty.Resolve(magnification, parentBox, buttonSize);
    SetPlacementBox(frame);
    m_boxArea.SetBox(frame);

    box2f const contentBox = m_text.Box();
    m_textPos = frame.Center() - contentBox.Center();
}
//=============================================================================
Window::Window()
    : Observer<ActivableMessage>(allow_destruction_of_observable)
    , m_sensitiveArea()
    , m_buttonList()
    , m_minClientSize(ui::Unit(30),ui::Unit(10))
    , m_maxClientSize(ui::Unit(std::numeric_limits<float>::infinity()), ui::Unit(std::numeric_limits<float>::infinity()))
    , m_requestedClientBox(box2f::FromMinDelta(float2(0),float2(0)))
    , m_currentClientBox(box2f::FromMinDelta(float2(0),float2(0)))
    , m_isManipulated(false)
    , m_pointedArea(Area::Outside)
    , m_clickedArea(Area::Outside)
    , m_prevPointerPosition()
{
    Common const& common = Common::Get();
    {
        ui::HorizontalListLayout::Properties prop;
        prop.margins.left      = ui::Magnifiable(0);
        prop.margins.top       = ui::Magnifiable(0);
        prop.margins.right     = ui::Magnifiable(0);
        prop.margins.bottom    = ui::Magnifiable(0);
        prop.margins.interItem = ui::Magnifiable(0);
        prop.widthFitMode = ui::FitMode::FitToContentOnly;
        m_buttonList = new ui::HorizontalListLayout(common.GetMagnifier(), prop);
    }
    {
        ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
        ui::Length const windowThickness = styleGuide.GetLength(common.MinManipulationThickness);
        ui::FrameProperty frameProp;
        frameProp.offset = ui::Length2(-windowThickness, windowThickness);
        frameProp.size = ui::Unit(float2(0));
        frameProp.alignmentToAnchor = float2(1,0);
        frameProp.anchorAlignment = float2(1,0);
        ui::FitMode2 fitMode(ui::FitMode::FitToContentOnly, ui::FitMode::FitToContentOnly);
        ui::FramingContainer* c = new ui::FramingContainer(common.GetMagnifier(), frameProp, fitMode);
        RequestAddToBack(c);
        c->SetContent(m_buttonList.get());
    }
    {
        WindowButton* b = new WindowButton(WindowButton::Type::Close);
        m_buttons[static_cast<int>(WindowButton::Type::Close)] = b;
        b->RegisterObserver(this);
    }
    UdateVisibleButtons();

    m_subContainers[0] = new ui::SubContainer;
    m_subContainers[1] = new ui::SubContainer;
    RequestAddToBack(m_subContainers[0].get());
    RequestAddToBack(m_subContainers[1].get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Window::~Window()
{
    m_subContainers[0] = nullptr;
    m_subContainers[1] = nullptr;
    m_contents[0] = nullptr;
    m_contents[1] = nullptr;
    m_buttonList = nullptr;
    RequestRemoveAll();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetTitle(std::wstring const& iTitle)
{
    Label* l = new Label(iTitle);
    m_subContainers[BAR]->RequestRemoveAll();
    m_contents[BAR] = l;
    m_subContainers[BAR]->RequestAddToBack(l);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetInfo(std::wstring const& iInfo)
{
    m_infoWindow.Show(false);
    m_infoWindow.ReleaseWindow_AssumeNotShown();
    if(!iInfo.empty())
    {
        Window* w = new Window;
        w->SetTitle(L"Info");
        w->SetContent(new Label(iInfo));
        w->SetClientSize(float2(200, 20));
        w->SetClientOffset(float2(60, 60));
        w->SetMinClientSize_RelativeToContent(ui::Relative(float2(1,1)));
        m_infoWindow.SetWindow_AssumeNone(w);

        if(nullptr == m_buttons[static_cast<int>(WindowButton::Type::Info)])
        {
            WindowButton* b = new WindowButton(WindowButton::Type::Info);
            m_buttons[static_cast<int>(WindowButton::Type::Info)] = b;
            b->RegisterObserver(this);
            UdateVisibleButtons();
        }
    }
    else
    {
        m_buttons[static_cast<int>(WindowButton::Type::Info)] = nullptr;
        UdateVisibleButtons();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::UdateVisibleButtons()
{
    m_buttonList->RemoveAllItems();
    if(nullptr != m_buttons[static_cast<int>(WindowButton::Type::Info)])
        m_buttonList->AppendItem(m_buttons[static_cast<int>(WindowButton::Type::Info)].get());
    if(nullptr != m_buttons[static_cast<int>(WindowButton::Type::Close)])
        m_buttonList->AppendItem(m_buttons[static_cast<int>(WindowButton::Type::Close)].get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetContent(ui::Component* iContent)
{
    m_subContainers[CONTENT]->RequestRemoveAll();
    m_contents[CONTENT] = iContent;
    m_subContainers[CONTENT]->RequestAddToBack(iContent);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetMinClientSize_RelativeToContent(ui::Length2 const& iMinSize)
{
    SG_CODE_FOR_ASSERT(CheckConstraintsOnMinOrMax(iMinSize));
    m_minClientSize = iMinSize;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetMaxClientSize_RelativeToContent(ui::Length2 const& iMaxSize)
{
    SG_CODE_FOR_ASSERT(CheckConstraintsOnMinOrMax(iMaxSize));
    m_maxClientSize = iMaxSize;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetClientSize(float2 const& iSize)
{
    m_requestedClientBox = box2f::FromMinDelta(m_requestedClientBox.Min(), iSize);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::SetClientOffset(float2 const& iOffset)
{
    m_requestedClientBox = box2f::FromMinDelta(iOffset, m_requestedClientBox.Delta());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SG_ENABLE_ASSERT
void Window::CheckConstraintsOnMinOrMax(ui::Length2 const& iMinOrMax)
{
    SG_ASSERT_MSG(0.f == iMinOrMax.x().relative || iMinOrMax.x() == ui::Relative(1.f), "if relative, a constraints can only be equal to content size");
    SG_ASSERT_MSG(0.f == iMinOrMax.y().relative || iMinOrMax.y() == ui::Relative(1.f), "if relative, a constraints can only be equal to content size");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::CheckCrossConstraintsOnMinMax()
{
    SG_ASSERT_MSG(     (  m_minClientSize.x().relative < m_maxClientSize.x().relative
                    || m_minClientSize.x().unit < m_maxClientSize.x().unit
                    || m_minClientSize.x().magnifiable < m_maxClientSize.x().magnifiable)
               || m_minClientSize.x() == m_maxClientSize.x(),
               "max size sould not be always smaller than min size");
    SG_ASSERT_MSG(     (  m_minClientSize.y().relative < m_maxClientSize.y().relative
                    || m_minClientSize.y().unit < m_maxClientSize.y().unit
                    || m_minClientSize.y().magnifiable < m_maxClientSize.y().magnifiable)
               || m_minClientSize.y() == m_maxClientSize.y(),
               "max size sould not be always smaller than min size");
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::CheckConstraintsOnContent(ui::IMovable* iContent)
{

}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::VirtualOnNotified(ActivableMessage const* iMessage)
{
    u32 const message = iMessage->GetMessage().data;
    switch(static_cast<WindowButton::Type>(message))
    {
    case WindowButton::Type::Close: Parent()->RequestRemove(this); break;
    case WindowButton::Type::MinimizeWindow: SG_ASSERT_NOT_REACHED(); break;
    case WindowButton::Type::MaximizeWidow:  SG_ASSERT_NOT_REACHED(); break;
    case WindowButton::Type::RestoreWindow:  SG_ASSERT_NOT_REACHED(); break;
    case WindowButton::Type::Info: m_infoWindow.Show(!m_infoWindow.IsShown()); break;
    default: SG_ASSUME_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool Window::VirtualIsInArea(float2 const& iPos) const
{
    box2f const& windowBox = PlacementBox_AssumeUpToDate();
    box2f const& clientBox = m_subContainers[CONTENT]->PlacementBox_AssumeUpToDate();
    box2f const& barBox = m_subContainers[BAR]->PlacementBox_AssumeUpToDate();
    SG_ASSERT(barBox.Min().x() == clientBox.Min().x());
    SG_ASSERT(barBox.Max().x() == clientBox.Max().x());

    float2 const windowSize = windowBox.Delta();
    float const borderSize = clientBox.Min().x() - windowBox.Min().x();
    float const cornerSize = 3 * borderSize;
    box2f const cornerBox = box2f::FromCenterDelta(windowBox.Center(), componentwise::max(windowSize - 2*cornerSize, float2(0.f)));

    if(windowBox.Contains(iPos))
    {
        if(clientBox.Min().x() <= iPos.x() && iPos.x() <= clientBox.Max().x() && barBox.Min().y() <= iPos.y() && iPos.y() <= clientBox.Max().y())
        {
            if(iPos.y() <= clientBox.Min().y())
                m_pointedArea = Area::Bar;
            else
                m_pointedArea = Area::Background;
        }
        else
        {
            if(iPos.y() <= cornerBox.Min().y())
            {
                if(iPos.x() <= cornerBox.Min().x())
                    m_pointedArea = Area::TopLeft;
                else if(cornerBox.Max().x() <= iPos.x())
                    m_pointedArea = Area::TopRight;
                else
                    m_pointedArea = Area::Top;
            }
            else if(cornerBox.Max().y() <= iPos.y())
            {
                if(iPos.x() <= cornerBox.Min().x())
                    m_pointedArea = Area::BottomLeft;
                else if(cornerBox.Max().x() <= iPos.x())
                    m_pointedArea = Area::BottomRight;
                else
                    m_pointedArea = Area::Bottom;
            }
            else
            {
                if(iPos.x() <= clientBox.Min().x())
                    m_pointedArea = Area::Left;
                else
                    m_pointedArea = Area::Right;
            }
        }
        return true;
    }
    else
    {
        m_pointedArea = Area::Outside;
        return false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent)
{
    parent_type::VirtualOnPointerEvent(iContext, iPointerEvent);
    m_sensitiveArea.OnPointerEvent(iContext, iPointerEvent, *this, *this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    SG_ASSERT(&m_sensitiveArea == iSensitiveArea);
    if(0 == iButton)
    {
        MoveToFrontOfAllUI();
        SG_ASSERT(m_pointedArea != Area::Outside);
        m_clickedArea = m_pointedArea;
        m_prevPointerPosition = iPointerLocalPosition.xy();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnDrag(ui::PointerEventContext const& iContext, ui::PointerEvent const& iEvent, float3 const& iPointerLocalPosition, size_t iButton)
{
    auto OnManipulate = [&]() {
        if(!m_isManipulated)
        {
            m_isManipulated = true;
            m_requestedClientBox = m_currentClientBox;
        }
    };
    if(0 == iButton)
    {
        float2 const d = iPointerLocalPosition.xy() - m_prevPointerPosition;
        m_prevPointerPosition = iPointerLocalPosition.xy();
        switch(m_clickedArea)
        {
        case Area::Outside: SG_ASSUME_NOT_REACHED();
        case Area::Background: break;
        case Area::Bar:         OnManipulate(); m_requestedClientBox += box2f::FromMinMax(d, d); InvalidatePlacement(); break;
        case Area::Left:        OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(d.x(), 0), float2(0)); InvalidatePlacement(); break;
        case Area::Top:         OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(0, d.y()), float2(0)); InvalidatePlacement(); break;
        case Area::Right:       OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(0), float2(d.x(), 0)); InvalidatePlacement(); break;
        case Area::Bottom:      OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(0), float2(0, d.y())); InvalidatePlacement(); break;
        case Area::TopLeft:     OnManipulate(); m_requestedClientBox += box2f::FromMinMax(d, float2(0)); InvalidatePlacement(); break;
        case Area::TopRight:    OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(0, d.y()), float2(d.x(), 0)); InvalidatePlacement(); break;
        case Area::BottomLeft:  OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(d.x(), 0), float2(0, d.y())); InvalidatePlacement(); break;
        case Area::BottomRight: OnManipulate(); m_requestedClientBox += box2f::FromMinMax(float2(0), d); InvalidatePlacement(); break;
        default:
            SG_ASSUME_NOT_REACHED();
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnPointerLeaveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    OnDrag(iContext, iEvent, iPointerLocalPosition, iButton);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::EndManipulationIFN()
{
    if(m_isManipulated)
    {
        m_requestedClientBox = m_currentClientBox;
        m_isManipulated = false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    if(0 == iButton) EndManipulationIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnButtonDownToUpOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON)
{
    if(0 == iButton) EndManipulationIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::OnReset(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_RESET)
{
    EndManipulationIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::DrawArea(ui::DrawContext const& iContext, Area iArea, bool iClicked) const
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();

    float const windowThickness = styleGuide.GetLength(common.MinManipulationThickness).Resolve(magnification, std::numeric_limits<float>::lowest());
    float const lineThickness = styleGuide.GetLength(common.WindowLineThickness).Resolve(magnification, std::numeric_limits<float>::lowest());
    //float const manipulatorThickness = styleGuide.GetLength(common.LineThickness1).Resolve(magnification, std::numeric_limits<float>::lowest());
    float const e = windowThickness - lineThickness; // manipulatorThickness; // windowThickness;

    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    box2f const& windowBox = PlacementBox_AssumeUpToDate();
    box2f const& extBox = windowBox + box2f::FromMinMax(float2(windowThickness-lineThickness), float2(-windowThickness+lineThickness));
    box2f const& clientBox = m_subContainers[CONTENT]->PlacementBox_AssumeUpToDate();
    box2f const& barBox = m_subContainers[BAR]->PlacementBox_AssumeUpToDate();
    SG_ASSERT(barBox.Min().x() == clientBox.Min().x());
    SG_ASSERT(barBox.Max().x() == clientBox.Max().x());

    float2 const windowSize = windowBox.Delta();
    float const borderSize = clientBox.Min().x() - windowBox.Min().x();
    float const cornerSize = 3 * borderSize;
    box2f const cornerBox = box2f::FromCenterDelta(windowBox.Center(), componentwise::max(windowSize - 2*cornerSize, float2(0.f)));

    Color4f const color = styleGuide.GetLinearColor(common.LineColorA1);
    switch(iArea)
    {
    case Area::Outside: SG_ASSUME_NOT_REACHED();
    case Area::Background: break;
    case Area::Bar: break;
    case Area::Left:
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Min()-float2(e,0), extBox.Corner(BitSet<2>(2))), color);
        break;
    case Area::Top:
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Min()-float2(0,e), extBox.Corner(BitSet<2>(1))), color);
        break;
    case Area::Right:
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Corner(BitSet<2>(1)), extBox.Max()+float2(e,0)), color);
        break;
    case Area::Bottom:
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Corner(BitSet<2>(2)), extBox.Max()+float2(0,e)), color);
        break;
    case Area::TopLeft:
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Min()-float2(e), float2(cornerBox.Min().x(), extBox.Min().y())), color);
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Min()-float2(e,0), float2(extBox.Min().x(), cornerBox.Min().y())), color);
        break;
    case Area::TopRight:
        drawer->DrawQuad(iContext, box2f::FromMinMax(float2(cornerBox.Max().x(), extBox.Min().y()-e), extBox.Corner(BitSet<2>(1))+float2(e,0)), color);
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Corner(BitSet<2>(1)), float2(extBox.Max().x()+e, cornerBox.Min().y())), color);
        break;
    case Area::BottomLeft:
        drawer->DrawQuad(iContext, box2f::FromMinMax(float2(extBox.Min().x()-e, cornerBox.Max().y()), extBox.Corner(BitSet<2>(2))+float2(0,e)), color);
        drawer->DrawQuad(iContext, box2f::FromMinMax(extBox.Corner(BitSet<2>(2)), float2(cornerBox.Min().x(), extBox.Max().y()+e)), color);
        break;
    case Area::BottomRight:
        drawer->DrawQuad(iContext, box2f::FromMinMax(float2(cornerBox.Max().x(), extBox.Max().y()), extBox.Max()+float2(e)), color);
        drawer->DrawQuad(iContext, box2f::FromMinMax(float2(extBox.Max().x(), cornerBox.Max().y()), extBox.Max()+float2(e,0)), color);
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::VirtualOnDraw(ui::DrawContext const& iContext)
{
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();

    box2f const& placementBox = PlacementBox();
    box2f const box = placementBox;
    box2f const barBox = m_subContainers[BAR]->PlacementBox();
    box2f const clientBox = m_subContainers[CONTENT]->PlacementBox();
    ui::UniformDrawer const* drawer = styleGuide.GetUniformDrawer(common.Default);
    float const lineThickness = styleGuide.GetLength(common.WindowLineThickness).Resolve(magnification, std::numeric_limits<float>::lowest());
    Color4f const barcolor = styleGuide.GetLinearColor(common.FillColorA1);
    if(AllGreaterStrict(barBox.Delta(), float2(0)))
        drawer->DrawQuad(iContext, barBox, barcolor);

    Color4f const bgcolor = styleGuide.GetLinearColor(common.FillColorA);
    if(AllGreaterStrict(clientBox.Delta(), float2(0)))
        drawer->DrawQuad(iContext, clientBox, bgcolor);

    Color4f const lineColor = styleGuide.GetLinearColor(common.LineColorA);
    //{
    //    box2f const inBox = box2f::FromMinMax(box.Min() + lineThickness, box.Max() - lineThickness);
    //    box2f const outBox = box;
    //    drawer->DrawFrame(iContext, inBox, outBox, lineColor);
    //}

    {
        box2f const inBox = box2f::FromMinMax(barBox.Min(), clientBox.Max());
        box2f const outBox = box2f::FromMinMax(inBox.Min() - lineThickness, inBox.Max() + lineThickness);
        if(AllGreaterStrict(inBox.Delta(), float2(0)))
            drawer->DrawFrame(iContext, inBox, outBox, lineColor);
    }
    {
        box2f const lineBox = box2f::FromMinMax(clientBox.Min() - float2(0,lineThickness), barBox.Max());
        if(AllGreaterStrict(lineBox.Delta(), float2(0)))
            drawer->DrawQuad(iContext, lineBox, lineColor);
    }
    if(IsDragged())
        DrawArea(iContext, m_clickedArea, true);
    else if(IsHover())
        DrawArea(iContext, m_pointedArea, false);

    parent_type::VirtualOnDraw(iContext);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Window::VirtualUpdatePlacement()
{
    SG_CODE_FOR_ASSERT(CheckCrossConstraintsOnMinMax());
    Common const& common = Common::Get();
    ui::GenericStyleGuide const& styleGuide = *common.StyleGuide();
    float const magnification = common.GetMagnifier().Magnification();
    box2f const& parentBox = Parent()->PlacementBox();

    box2f const prevWindowBox = PlacementBoxAsIs_AssumeInUpdatePlacement();

    float const lineThickness = styleGuide.GetLength(common.WindowLineThickness).Resolve(magnification, std::numeric_limits<float>::lowest());
    float const windowThickness = styleGuide.GetLength(common.MinManipulationThickness).Resolve(magnification, std::numeric_limits<float>::lowest());

    float2 const minClientSize0 = m_minClientSize.Resolve(magnification, float2(0));

    float2 prevClientSize = componentwise::max(minClientSize0, m_requestedClientBox.Delta());
    float2 prevWindowSize = prevWindowBox.Delta();
    SG_CODE_FOR_ASSERT(size_t iter_count = 0;)
    for(;;)
    {
        SetPlacementBox(box2f::FromMinMax(float2(0), float2(0)));
        float2 const contentSize = [&]() {
            if(nullptr != m_contents[CONTENT])
            {
                m_subContainers[CONTENT]->SetPlacementBox(box2f::FromMinMax(float2(0), prevClientSize));
                box2f const contentBox = m_contents[CONTENT]->PlacementBox();
                return contentBox.Delta();
            }
            else
                return float2(0);
        } ();
        float2 const barContentSize = [&]() {
            if(nullptr != m_contents[BAR])
            {
                m_subContainers[BAR]->SetPlacementBox(box2f::FromMinMax(float2(0), float2(prevClientSize.x(), 0)));
                box2f const contentBox = m_contents[BAR]->PlacementBox();
                return contentBox.Delta();
            }
            else
                return float2(0);
        } ();

        float2 const barOffset = float2(windowThickness);
        float2 const clientOffset = barOffset + float2(0, barContentSize.y());
        float2 const clientToWindowSizeDiff = 2 * windowThickness + float2(0, barContentSize.y());
        float2 const maxWindowSize = parentBox.Delta();
        float2 const maxClientSizeFromWindow = maxWindowSize - clientToWindowSizeDiff;
        float2 const minClientSize = m_minClientSize.Resolve(magnification, contentSize);
        float2 const maxClientSize0 = m_maxClientSize.Resolve(magnification, contentSize);
        float2 const maxClientSize = componentwise::min(maxClientSize0, maxClientSizeFromWindow);

        float2 const clientSize = componentwise::max(minClientSize, componentwise::min(maxClientSize, prevClientSize));
        float2 const windowSize = clientSize + clientToWindowSizeDiff;
        float2 const barSize = float2(clientSize.x(), barContentSize.y());

        if(EqualsWithTolerance(clientSize, prevClientSize, 0.01f) && EqualsWithTolerance(windowSize, prevWindowSize, 0.01f))
        {
            float2 clientOffsetToParent = m_requestedClientBox.Min();
            if(m_isManipulated)
            {
                float2 requestedMove = m_requestedClientBox.Min() - m_currentClientBox.Min();
                float2 constraintOnMove = m_requestedClientBox.Delta() - clientSize;
                if(0 != requestedMove.x()) clientOffsetToParent.x() += constraintOnMove.x();
                if(0 != requestedMove.y()) clientOffsetToParent.y() += constraintOnMove.y();
            }
            float2 const windowOffset = clientOffsetToParent - clientOffset;
            float2 const windowMin = parentBox.Min() + windowOffset;
            float2 const windowMax = windowMin + windowSize;
            // TODO: Forbid Window to go "outside" its container.
            SetPlacementBox(box2f::FromMinDelta(windowMin, windowSize));
            box2f const clientBox = box2f::FromMinDelta(windowMin+clientOffset, clientSize);
            m_subContainers[CONTENT]->SetPlacementBox(clientBox);
            m_subContainers[BAR]->SetPlacementBox(box2f::FromMinDelta(windowMin+barOffset, barSize));
            m_currentClientBox = clientBox;
            break;
        }

        prevWindowSize = windowSize;
        prevClientSize = clientSize;
        SG_ASSERT(++iter_count < 3);
    }
}
//=============================================================================
}
}
#endif
