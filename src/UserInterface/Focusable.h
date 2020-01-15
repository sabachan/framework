#ifndef UserInterface_Focusable_H
#define UserInterface_Focusable_H

#include "Component.h"

#include <System/UserInputEvent.h>

// The focus indicates which components will receive the events not associated
// to a position like keyboard events.
// The focus can be modelled as a chain of components.
// For instance, if the focus is own by a button in a window, some event will
// be interpreted by the button, others will be by the window and others by
// a global system.
//
// Some virtual functions *must* be overloaded when one wants to implement a
// focusable component. Specificaly, methods VirtualAsComponent() and
// VirtualAsFocusableIFP() are needed to cast back and forth, and
// VirtualOnInsertInUI() and VirtualOnRemoveFromUI() must forward to the
// focusable methods.
// Here is a minimal focusable component implementation:
//
//    class MyFocusaleComponent : public ui::Component
//                              , private ui::IFocusable
//    {
//        typedef ui::Component parent_type;
//    protected:
//        virtual ui::Component* VirtualAsComponent() override { return this; }
//        virtual ui::IFocusable* VirtualAsFocusableIFP() override { return this; }
//    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
//    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }
//    };
//
// 
// IFocusable offers some virtual methods for the derived classes to implement.
// - VirtualOnFocusableEvent() is used to interpret events.
// - VirtualMoveFocusReturnHasMoved() can be implemented to specify how the
//          focus moves from one child to another.
// - VirtualOnChangeFocusChild() may be used to release its own focus when
//          child is null, for a container. It can also be used to implement
//          auto-scrolling.

namespace sg {
namespace ui {
//=============================================================================
class FocusableEvent
{
    SG_NON_COPYABLE(FocusableEvent)
public:
    FocusableEvent(system::UserInputEvent const& iEvent SG_CODE_FOR_ASSERT(SG_COMMA size_t iIndex))
        : m_event(&iEvent)
        SG_CODE_FOR_ASSERT(SG_COMMA m_index(iIndex))
    {}
    ~FocusableEvent() {}

    system::UserInputEvent const& Event() const { return *m_event; }
#if SG_ENABLE_ASSERT
    size_t Index() const { return m_index; }
#endif
private:
    SG_CODE_FOR_ASSERT(size_t m_index);
    safeptr<system::UserInputEvent const> m_event;
};
//=============================================================================
enum class FocusDirection
{
    None,
    Left,
    Right,
    Up,
    Down,
    Next,
    Previous,
    MoveIn,
    MoveOut,
    Activate,
    Validate,
    Cancel,
};
//=============================================================================
class IFocusable : public SafeCountable
{
    friend class RootContainer;
public:
    IFocusable();
    ~IFocusable();
    void OnFocusableEvent(FocusableEvent const& iFocusableEvent);
    void RequestFocusIFN();
    void RequestTerminalFocusIFN();
    void ReleaseFocusIFN();
    bool RequestMoveFocusReturnHasMoved(FocusDirection iDirection);
    bool HasFocus() const { return nullptr != m_focusParent; }
    bool HasTerminalFocus() const { return nullptr != m_focusParent && nullptr == m_focusChild; }
    IFocusable* FocusParent() const { SG_ASSERT((nullptr != m_focusParent) != m_isRoot); return m_focusParent.get(); }
    IFocusable* FocusChild() const { return m_focusChild.get(); }
    Component* AsComponent() { Component* c = VirtualAsComponent(); SG_ASSERT(nullptr != c); return c; }
    SG_CODE_FOR_ASSERT(bool IsInGUIForAssert() const { return m_isInGUI; })
protected:
    void RequestFocus();
    virtual Component* VirtualAsComponent() = 0;
    virtual void VirtualOnFocusableEvent(FocusableEvent const& iFocusableEvent);
    virtual bool VirtualMoveFocusReturnHasMoved(FocusDirection iDirection);
    virtual void VirtualOnSetFocusChild(IFocusable* iFocusable);
    virtual void VirtualOnGainFocus();
    virtual void VirtualOnLooseFocus();
    void OnInsertFocusableInUI();
    void OnRemoveFocusableFromUI();
private:
    IFocusable(ComponentIsRoot_t);
    void SetFocusChild(IFocusable* iFocusable);
    void OnSetFocusChild(IFocusable* iFocusable);
    void OnGainFocus();
    void OnLooseFocus();
private:
    safeptr<IFocusable> m_focusParent;
    safeptr<IFocusable> m_focusChild;
    SG_CODE_FOR_ASSERT(bool const m_isRoot;)
    SG_CODE_FOR_ASSERT(bool m_virtualSetChildCalled { false };)
    SG_CODE_FOR_ASSERT(bool m_isInSetFocusChild { false };)
    SG_CODE_FOR_ASSERT(bool m_virtualEventCalled { false };)
    SG_CODE_FOR_ASSERT(bool m_isInGUI { false };)
};
//=============================================================================
}
}

#endif
