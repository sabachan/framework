#ifndef UserInterface_PointerEventClipper_H
#define UserInterface_PointerEventClipper_H

#include "Area.h"
#include "PointerEvent.h"
#include <System/UserInputEvent.h>

namespace sg {
namespace ui {
//=============================================================================
class PointerEventClipper : private system::UserInputEventClipper
{
    SG_NON_COPYABLE(PointerEventClipper)
    SG_NON_NEWABLE
public:
    PointerEventClipper(PointerEventContext const& iContext, PointerEvent const& iPointerEvent, IArea const& iArea)
        : UserInputEventClipper(iPointerEvent.Event(), MustBeClipped(iContext, iPointerEvent, iArea))
    {
    }
    ~PointerEventClipper()
    {
    }
private:
    static bool MustBeClipped(PointerEventContext const& iContext, PointerEvent const& iPointerEvent, IArea const& iArea)
    {
        system::UserInputEvent const& event = iPointerEvent.Event();
        if(system::UserInputDeviceType::Mouse == event.DeviceType())
        {
            if(system::UserInputEventType::ResetDevice != event.EventType()
               && system::UserInputEventType::ResetEntry != event.EventType())
            {
                float3 pointerLocalPosition(uninitialized);
                bool const isInside = IsInside_AssumePointerEvent(iContext, event, iArea, pointerLocalPosition);
                return !isInside;
            }
        }
        return false;
    }
};
//=============================================================================
}
}

#endif
