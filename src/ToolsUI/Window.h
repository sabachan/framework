#ifndef ToolsUI_Window_H
#define ToolsUI_Window_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include <UserInterface/Container.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/ListLayout.h>
#include <UserInterface/Movable.h>
#include <UserInterface/SensitiveArea.h>
#include <Core/Observer.h>

namespace sg {
namespace toolsui {
//=============================================================================
struct Message
{
    u32 data;
    Message(u32 iData) : data(iData) {}
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ActivableMessage : public UnsharableObservable<ActivableMessage>
{
public:
    ActivableMessage(Message iMessage) : m_message(iMessage) {}
    Message GetMessage() const { return m_message; }
private:
    Message m_message;
};
//=============================================================================
class Window;
class WindowButton;
//=============================================================================
class WindowCloseEvent : public UnsharableObservable<WindowCloseEvent> {};
//=============================================================================
template<bool IsModal>
class WindowHandlerImpl : private Observer<WindowCloseEvent>
{
public:
    WindowHandlerImpl();
    WindowHandlerImpl(Window* iWindow);
    ~WindowHandlerImpl();
    void Show(bool iValue);
    bool IsShown();
    void ReleaseWindow_AssumeNotShown();
    void SetWindow_AssumeNone(Window* iWindow);
private:
    virtual void VirtualOnNotified(WindowCloseEvent const* iCloseEvent) override;
private:
    refptr<Window> m_window;
    bool m_shown;
};
typedef WindowHandlerImpl<false> WindowHandler;
typedef WindowHandlerImpl<true> ModalWindowHandler;
//=============================================================================
class Window final : public ui::Container
                   , private ui::ISensitiveAreaListener
                   , private ui::IArea
                   , private Observer<ActivableMessage>
{
    typedef ui::Container parent_type;
    enum class Area
    {
        Outside,
        Background,
        Bar,
        Left, Top, Right, Bottom,
        TopLeft, TopRight, BottomLeft, BottomRight,
    };
public:
    //struct Properties
    //{
    //    bool minimizable;
    //    bool maximizable;
    //    bool closable;
    //};
public:
    Window();
    ~Window();
    void SetTitle(std::wstring const& iTitle);
    void SetInfo(std::wstring const& iInfo);
    void SetContent(ui::Component* iContent);
    void SetMinClientSize_RelativeToContent(ui::Length2 const& iMinSize);
    void SetMaxClientSize_RelativeToContent(ui::Length2 const& iMaxSize);
    void SetClientSize(float2 const& iSize);
    void SetClientOffset(float2 const& iOffset);
    WindowCloseEvent const& GetCloseEvent() const { return m_closeEvent; }
private:
    bool IsHover() const { return m_sensitiveArea.IsPointerInsideFree(); }
    bool IsClicked() const { return m_sensitiveArea.IsClickedValidable(0); }
    bool IsDragged() const { return m_sensitiveArea.IsClicked(0); }
    virtual void VirtualOnNotified(ActivableMessage const* iMessage) override;
    virtual bool VirtualIsInArea(float2 const& iPos) const override;
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerLeaveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUpOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnReset(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_RESET) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnChildInvalidatePlacement() override { InvalidatePlacement(); }
    virtual void VirtualOnRemoveFromUI() override { parent_type::VirtualOnRemoveFromUI(); m_closeEvent.NotifyObservers(); }
    void DrawArea(ui::DrawContext const& iContext, Area iArea, bool iClicked) const;
    void OnDrag(ui::PointerEventContext const& iContext, ui::PointerEvent const& iEvent, float3 const& iPointerLocalPosition, size_t iButton);
    void EndManipulationIFN();
    void UdateVisibleButtons();
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnMinOrMax(ui::Length2 const& iMinOrMax);
    void CheckCrossConstraintsOnMinMax();
    void CheckConstraintsOnContent(ui::IMovable* iContent);
#endif
private:
    static size_t const CONTENT = 0;
    static size_t const BAR = 1;
private:
    ui::SensitiveArea m_sensitiveArea;
    std::wstring m_info;
    WindowHandler m_infoWindow;
    safeptr<ui::SubContainer> m_subContainers[2];
    safeptr<ui::Component> m_contents[2];
    safeptr<ui::HorizontalListLayout> m_buttonList;
    refptr<WindowButton> m_buttons[2];
    ui::Length2 m_minClientSize;
    ui::Length2 m_maxClientSize;
    box2f m_requestedClientBox;
    box2f m_currentClientBox;
    bool m_isManipulated;
    mutable Area m_pointedArea;
    Area m_clickedArea;
    float2 m_prevPointerPosition;
    WindowCloseEvent m_closeEvent;
};
//=============================================================================
}
}

#endif
