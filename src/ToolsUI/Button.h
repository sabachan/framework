#ifndef ToolsUI_Button_H
#define ToolsUI_Button_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include "Common.h"
#include <Core/Observer.h>
#include <Rendering/Color.h>
#include <UserInterface/Container.h>
#include <UserInterface/Component.h>
#include <UserInterface/FitMode.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/Movable.h>
#include <UserInterface/SensitiveArea.h>

namespace sg {
namespace toolsui {
//=============================================================================
class Label;
//=============================================================================
class Button : public ui::Container
             , public ui::IMovable
             , public UnsharableObservable<Button>
             , private ui::ISensitiveAreaListener
{
    typedef ui::Container parent_type;
public:
    Button();
    Button(ui::FitMode2 iFitMode);
    ~Button();
    void SetContent(ui::IMovable* iContent);
    void SetFrameProperty(ui::FrameProperty const& iFrameProperty)  { InvalidatePlacement(); m_frameProperty = iFrameProperty; }
    void SetFitMode(ui::FitMode2 iFitMode) { InvalidatePlacement(); m_fitMode = iFitMode; }
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
    ui::SensitiveArea const& SensitiveArea() const { return m_sensitiveArea; }
private:
    bool IsHover() const { return m_sensitiveArea.IsPointerInsideFree(); }
    bool IsClicked() const { return m_sensitiveArea.IsClickedValidable(0); }
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnContent(ui::IMovable* iContent);
#endif
private:
    ui::FrameProperty m_frameProperty;
    ui::SensitiveArea m_sensitiveArea;
    ui::BoxArea m_boxArea;
    safeptr<ui::SubContainer> m_subContainer;
    safeptr<ui::IMovable> m_content;
    ui::FitMode2 m_fitMode;
};
//=============================================================================
class TextButton : public Button
{
    typedef Button parent_type;
public:
    TextButton(std::wstring const& iText);
    ~TextButton();
    void SetText(std::wstring const& iText);
private:
    refptr<Label> m_label;
};
//=============================================================================
}
}

#endif
