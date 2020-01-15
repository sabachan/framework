#ifndef UserInterface_SensitiveArea_H
#define UserInterface_SensitiveArea_H

#include "Area.h"
#include <Core/BitSet.h>
#include <Math/Box.h>
#include <Math/Vector.h>
#include <System/MouseUtils.h>

namespace sg {
namespace ui {
//=============================================================================
class IArea;
class ISensitiveAreaListener;
class PointerEvent;
class PointerEventContext;
//=============================================================================
class SensitiveArea
{
public:
    static size_t const BUTTON_COUNT = 8;
public:
    struct PremaskState
    {
        friend SensitiveArea;
    public:
        PremaskState(system::MouseEntry iPremaskableEntry) : premaskableEntries(checked_numcastable(iPremaskableEntry)) {}
    private:
        // premaskableEntries is a union of values of system::MouseEntry
        u8 premaskableEntries;
        bool eventIsPremasked;
        bool isInsideArea;
        float3 pointerLocalPosition;
    #if SG_ENABLE_ASSERT
        enum class State { Begin, RunChildren, End };
        State stateForCheck { State::Begin };
    #endif
    };
public:
    SensitiveArea();
    void Reset(ISensitiveAreaListener& iListener);

    // Mode 1: Treat all events in the area and mask all of them.
    void OnPointerEvent(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea, ISensitiveAreaListener& iListener);

    // Mode 2: Treat only a subset of button events and allow these events to
    // be interpreted by a nested element in priority. In that case, the nested
    // element must also use premasking method.
    // However, even if events that are not premasked will not be masked, they
    // are still treated. It is however expected that the client doesn't use
    // them. This behavior may change in the future, so that such events are
    // not transfered to the listener.
    void OnPointerEventPreChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea, PremaskState& ioState);
    void OnPointerEventPostChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, PremaskState& iState, ISensitiveAreaListener& iListener);

    bool IsPointerInside() const { return m_pointerInside; }
    bool IsPointerInsideFree() const { return m_pointerInside && (all_ones == m_buttonDownIfOne); }
    bool IsClicked(size_t iButton) const { return m_buttonDownFromInside[iButton]; }
    bool IsClickedValidable(size_t iButton) const { return m_pointerInside && m_buttonDownFromInside[iButton]; }
    bool IsClickedNotValidable(size_t iButton) const { return !m_pointerInside && m_buttonDownFromInside[iButton]; }
private:
    template<bool PremaskMode>
    void OnPointerEventImpl(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const* iArea, PremaskState const* iState, ISensitiveAreaListener& iListener);
private:
    BitSet<BUTTON_COUNT> m_buttonDownFromInside;
    BitSet<BUTTON_COUNT> m_buttonDownFromOutside;
    u8 m_buttonDownIfOne;
    bool m_pointerInside;
    bool m_buttonsMessedUp;
    SG_CODE_FOR_ASSERT(size_t m_previousEventIndex);
};
//=============================================================================
#if SG_ENABLE_ASSERT
SG_DEFINE_TYPED_TAG(SensitiveAreaListener_PleaseUseMacroForParameters)
#endif
//=============================================================================
#define SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_FREE \
    SG_CODE_FOR_ASSERT(::sg::ui::SensitiveAreaListener_PleaseUseMacroForParameters_t /*no name*/ SG_COMMA) \
    ::sg::ui::SensitiveArea const* iSensitiveArea, \
    ::sg::ui::PointerEventContext const& iContext, \
    ::sg::ui::PointerEvent const& iEvent, \
    ::sg::float3 const& iPointerLocalPosition
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON \
    SG_CODE_FOR_ASSERT(::sg::ui::SensitiveAreaListener_PleaseUseMacroForParameters_t /*no name*/ SG_COMMA) \
    ::sg::ui::SensitiveArea const* iSensitiveArea, \
    ::sg::ui::PointerEventContext const& iContext, \
    ::sg::ui::PointerEvent const& iEvent, \
    ::sg::float3 const& iPointerLocalPosition, \
    size_t iButton
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_RESET \
    SG_CODE_FOR_ASSERT(::sg::ui::SensitiveAreaListener_PleaseUseMacroForParameters_t /*no name*/ SG_COMMA) \
    ::sg::ui::SensitiveArea const* iSensitiveArea
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_FREE \
    SG_CODE_FOR_ASSERT(::sg::ui::SensitiveAreaListener_PleaseUseMacroForParameters SG_COMMA) \
    iSensitiveArea, iContext, iEvent, iPointerLocalPosition
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_ONE_BUTTON \
    SG_CODE_FOR_ASSERT(::sg::ui::SensitiveAreaListener_PleaseUseMacroForParameters SG_COMMA) \
    iSensitiveArea, iContext, iEvent, iPointerLocalPosition, iButton
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_FORWARD_PARAMETERS_RESET \
    SG_CODE_FOR_ASSERT(::sg::ui::SensitiveAreaListener_PleaseUseMacroForParameters SG_COMMA) \
    iSensitiveArea
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_FREE \
    SG_UNUSED((iSensitiveArea, iContext, iEvent, iPointerLocalPosition))
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON \
    SG_UNUSED((iSensitiveArea, iContext, iEvent, iPointerLocalPosition, iButton))
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_RESET \
    SG_UNUSED((iSensitiveArea))
//=============================================================================
class ISensitiveAreaListener
{
public:
    virtual void OnPointerEnterFree(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_FREE) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_FREE; }
    virtual void OnPointerMoveFree(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_FREE) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_FREE; }
    virtual void OnPointerLeaveFree(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_FREE) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_FREE; }

    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnPointerEnterButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnPointerMoveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnPointerLeaveButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnPointerMoveOutsideButtonDownFromInside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnButtonDownToUpOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }

    virtual void OnPointerEnterButtonDownFromOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnPointerMoveButtonDownFromOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnPointerLeaveButtonDownFromOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }
    virtual void OnButtonDownToUpFromOutside(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_ONE_BUTTON; }

    virtual void OnReset(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_RESET) { SG_UI_SENSITIVE_AREA_LISTENER_UNUSED_PARAMETERS_RESET; }
};
//=============================================================================
}
}

#endif
