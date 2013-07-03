#include "stdafx.h"

#include "Area.h"

#include "Context.h"
#include "PointerEvent.h"
#include <System/MouseUtils.h>

namespace sg {
namespace ui {
//=============================================================================
bool IsInside_AssumePointerEvent(PointerEventContext const& iContext, system::UserInputEvent const& iEvent, IArea const& iArea, float3& oPosition)
{
    SG_ASSERT(system::UserInputDeviceType::Mouse == iEvent.DeviceType());
    SG_ASSERT(system::UserInputEventType::ResetDevice != iEvent.EventType());
    SG_ASSERT(system::UserInputEventType::ResetEntry != iEvent.EventType());
    SG_ASSERT(system::UserInputEventType::ResetAll != iEvent.EventType());
    SG_ASSERT(system::UserInputEventType::None != iEvent.EventType());
    system::MouseState const mouseState(iEvent.State());
    float2 const mousePos(mouseState.Position());
    iContext.TransformPointerPosition(oPosition, mousePos);
    float3 const position = oPosition;
    SG_ASSERT(1.f == position.z() || 0.f == position.z());
    if(0.f == position.z())
        return false;
    bool const isInside = iArea.VirtualIsInArea(position.xy());
    return isInside;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool IsInside_AssumePointerEvent(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea, float3& oPosition)
{
    system::UserInputEvent const& event = iEvent.Event();
    return IsInside_AssumePointerEvent(iContext, event, iArea, oPosition);
}
//=============================================================================
}
}
