#ifndef UserInterface_WheelSensitiveArea_H
#define UserInterface_WheelSensitiveArea_H

#include "SensitiveArea.h"
#include <Core/BitSet.h>
#include <Math/Box.h>
#include <Math/Vector.h>

namespace sg {
namespace ui {
//=============================================================================
class IWheelListener;
class PointerEvent;
class PointerEventContext;
//=============================================================================
class WheelHelper
{
public:
    WheelHelper();
    void OnPointerEventPreChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea);
    void OnPointerEventPostChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, IWheelListener& iListener);
private:
    bool m_wheelEventIsPremasked;
    float3 m_pointerLocalPosition;
#if SG_ENABLE_ASSERT
    enum class State { Begin, RunChildren, End };
    State m_stateForCheck;
#endif
};
//=============================================================================
#if SG_ENABLE_ASSERT
SG_DEFINE_TYPED_TAG(WheelListener_PleaseUseMacroForParameters)
#endif
//=============================================================================
#define SG_UI_WHEEL_LISTENER_PARAMETERS \
    SG_CODE_FOR_ASSERT(::sg::ui::WheelListener_PleaseUseMacroForParameters_t /*no name*/ SG_COMMA) \
    ::sg::ui::PointerEventContext const& iContext, \
    ::sg::ui::PointerEvent const& iEvent, \
    float const& iWheelDelta, \
    ::sg::float3 const& iPointerLocalPosition
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#define SG_UI_WHEEL_LISTENER_UNUSED_PARAMETERS \
    SG_UNUSED((iContext, iEvent, iWheelDelta, iPointerLocalPosition))
//=============================================================================
class IWheelListener
{
public:
    virtual void OnWheelEvent(SG_UI_WHEEL_LISTENER_PARAMETERS) = 0;
};
//=============================================================================
}
}

#endif
