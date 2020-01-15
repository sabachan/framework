#include "stdafx.h"
#include "WheelHelper.h"

#include "Context.h"
#include "PointerEvent.h"
#include <System/MouseUtils.h>

namespace sg {
namespace ui {
//=============================================================================
WheelHelper::WheelHelper()
    : m_wheelEventIsPremasked(false)
    , m_pointerLocalPosition()
    SG_CODE_FOR_ASSERT(SG_COMMA m_stateForCheck(State::Begin))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WheelHelper::OnPointerEventPreChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea)
{
    SG_ASSERT_MSG(State::Begin == m_stateForCheck, "This helper should be allocated on stack and used for one event only");
    SG_CODE_FOR_ASSERT(m_stateForCheck = State::RunChildren);
    SG_ASSERT(false == m_wheelEventIsPremasked);
    SG_ASSERT(float3(0) == m_pointerLocalPosition);
    system::UserInputEvent const& event = iEvent.Event();
    if(event.CanBePreMasked()
        && system::UserInputDeviceType::Mouse == event.DeviceType()
        && system::UserInputEventType::Message == event.EventType()
        && static_cast<u32>(system::MouseEntry::Wheel) == event.EntryId())
    {
        system::MouseState const mouseState(event.State());
        float2 const mousePos(mouseState.Position());
        iContext.TransformPointerPosition(m_pointerLocalPosition, mousePos);
        if(0.f != m_pointerLocalPosition.z())
        {
            bool const isInside = (0.f != m_pointerLocalPosition.z()) && iArea.VirtualIsInArea(m_pointerLocalPosition.xy());
            if(isInside)
            {
                event.SetPremasked();
                m_wheelEventIsPremasked = true;
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WheelHelper::OnPointerEventPostChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, IWheelListener& iListener)
{
#if SG_ENABLE_ASSERT
    WheelListener_PleaseUseMacroForParameters_t const dummyParameter = WheelListener_PleaseUseMacroForParameters;
    SG_ASSERT_MSG(State::RunChildren == m_stateForCheck, "This helper should be allocated on stack and used for one event only");
    SG_CODE_FOR_ASSERT(m_stateForCheck = State::End);
#endif
    if(m_wheelEventIsPremasked)
    {
        system::UserInputEvent const& event = iEvent.Event();
        SG_ASSERT(static_cast<u32>(system::MouseEntry::Wheel) == event.EntryId());
        if(event.CanBePreMasked())
        {
            SG_ASSERT(event.IsPreMasked());
            system::MouseState const mouseState(event.State());
            system::MouseWheelAdditionalData const additionalData(event.AdditionalData());
            float const delta = additionalData.WheelDeltaFP16() / float(1 << 16);
            iListener.OnWheelEvent(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) iContext, iEvent, delta, m_pointerLocalPosition);
            event.SetMaskedFromPremasked();
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void WheelHelper::OnPointerEventNoChild(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea, IWheelListener& iListener)
{
    OnPointerEventPreChildren(iContext, iEvent, iArea);
    OnPointerEventPostChildren(iContext, iEvent, iListener);
}
//=============================================================================
}
}
