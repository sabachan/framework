#ifndef ToolsUI_CheckBox_H
#define ToolsUI_CheckBox_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include <Core/Observer.h>
#include <UserInterface/Container.h>
#include <UserInterface/Component.h>
#include <UserInterface/FitMode.h>
#include <UserInterface/Focusable.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/ListLayout.h>
#include <UserInterface/Movable.h>
#include <UserInterface/SensitiveArea.h>
#include <UserInterface/Text.h>
#include "Common.h"

namespace sg {
namespace toolsui {
//=============================================================================
class Label;
//=============================================================================
class CheckBox : public ui::Container
               , public ui::IMovable
               , public UnsharableObservable<CheckBox>
               , private ui::IFocusable
               , private ui::ISensitiveAreaListener
{
    PARENT_SAFE_COUNTABLE(ui::Container)
    typedef ui::Container parent_type;
    typedef ui::IFocusable focusable_parent_type;
public:
    CheckBox();
    CheckBox(ui::FitMode2 iFitMode);
    ~CheckBox();
    void SetState(bool iState);
    void FlipState() { SetState(!m_state); }
    bool GetState() const { return m_state; }
    void SetContent(ui::IMovable* iContent);
    void SetFrameProperty(ui::FrameProperty const& iFrameProperty)  { InvalidatePlacement(); m_frameProperty = iFrameProperty; }
    void SetFitMode(ui::FitMode2 iFitMode) { InvalidatePlacement(); m_fitMode = iFitMode; }
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnFocusableEvent(ui::FocusableEvent const& iFocusableEvent) override;
    virtual bool VirtualMoveFocusReturnHasMoved(ui::FocusDirection iDirection) override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
    virtual IFocusable* VirtualAsFocusableIFP() override { return this; }
    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }
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
    safeptr<ui::HorizontalListLayout> m_list;
    safeptr<ui::IMovable> m_content;
    refptr<Label> m_checkBox;
    ui::FitMode2 m_fitMode;
    bool m_state;
};
//=============================================================================
class TextCheckBox : public CheckBox
{
    typedef CheckBox parent_type;
public:
    TextCheckBox(std::wstring const& iText);
    ~TextCheckBox();
    void SetText(std::wstring const& iText);
private:
    refptr<Label> m_label;
};
//=============================================================================
}
}

#endif
