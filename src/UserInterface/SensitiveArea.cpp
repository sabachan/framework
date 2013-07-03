#include "stdafx.h"

#include "SensitiveArea.h"

#include "Area.h"
#include "Context.h"
#include "PointerEvent.h"
#include <System/MouseUtils.h>

namespace sg {
namespace ui {
//=============================================================================
SensitiveArea::SensitiveArea()
    : m_buttonDownFromInside()
    , m_buttonDownFromOutside()
    , m_buttonDownIfOne(all_ones)
    , m_pointerInside(false)
    , m_buttonsMessedUp(false)
    SG_CODE_FOR_ASSERT(SG_COMMA m_previousEventIndex(0))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SensitiveArea::Reset(ISensitiveAreaListener& iListener)
{
#if SG_ENABLE_ASSERT
    SensitiveAreaListener_PleaseUseMacroForParameters_t const dummyParameter = SensitiveAreaListener_PleaseUseMacroForParameters;
#endif
    SG_CODE_FOR_ASSERT(m_previousEventIndex = 0;)
    m_buttonsMessedUp = false;
    m_pointerInside = false;
    m_buttonDownFromInside.reset();
    m_buttonDownFromOutside.reset();
    m_buttonDownIfOne = all_ones;
    iListener.OnReset(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SensitiveArea::OnPointerEventPreChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const& iArea, PremaskState& ioState)
{
#if SG_ENABLE_ASSERT
    bool const mayBeAResetEvent = 0 == iEvent.Index();
    SG_ASSERT_MSG(0 == m_previousEventIndex || mayBeAResetEvent || m_previousEventIndex+1 == iEvent.Index(), "All events must be transmitted to sensitive area as long as it is not reset.");
    m_previousEventIndex = iEvent.Index();
    SG_ASSERT_MSG(PremaskState::State::Begin == ioState.stateForCheck, "PremaskState should be instanciated on the stack.");
    ioState.stateForCheck = PremaskState::State::RunChildren;
#endif
    ioState.eventIsPremasked = false;
    ioState.isInsideArea = false;
    ioState.pointerLocalPosition = float3(0);
    system::UserInputEvent const& event = iEvent.Event();
    if(system::UserInputDeviceType::Mouse == event.DeviceType())
    {
        if(system::UserInputEventType::ResetDevice != event.EventType()
           && system::UserInputEventType::ResetEntry != event.EventType())
        {
            system::MouseState const mouseState(event.State());
            u8 const mouseButtonsDown = checked_numcastable(mouseState.GetMouseButtonsDown() | event.EntryId());
            if(0 != (mouseButtonsDown & ioState.premaskableEntries))
            {
                float3 localPointerPosition(0);
                bool const isInsideArea = IsInside_AssumePointerEvent(iContext, event, iArea, localPointerPosition);
                if(isInsideArea)
                {
                    if(event.CanBePreMasked())
                    {
                        event.SetPremasked();
                        ioState.eventIsPremasked = true;
                    }
                }

                ioState.isInsideArea = isInsideArea;
                ioState.pointerLocalPosition = localPointerPosition;
            }
        }
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SensitiveArea::OnPointerEventPostChildren(PointerEventContext const& iContext, PointerEvent const& iEvent, PremaskState& iState, ISensitiveAreaListener& iListener)
{
#if SG_ENABLE_ASSERT
    SG_ASSERT_MSG(m_previousEventIndex == iEvent.Index(), "Did you forgot to call OnPointerEventPreChildren?");
    SG_ASSERT(PremaskState::State::RunChildren == iState.stateForCheck);
    iState.stateForCheck = PremaskState::State::End;
#endif
    OnPointerEventImpl<true>(iContext, iEvent, nullptr, &iState, iListener);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SensitiveArea::OnPointerEvent(PointerEventContext const& iContext,
                                   PointerEvent const& iEvent,
                                   IArea const& iArea,
                                   ISensitiveAreaListener& iListener)
{
#if SG_ENABLE_ASSERT
    bool const mayBeAResetEvent = 0 == iEvent.Index();
    SG_ASSERT_MSG(0 == m_previousEventIndex || mayBeAResetEvent || m_previousEventIndex+1 == iEvent.Index(), "All events must be transmitted to sensitive area as long as it is not reset.");
    m_previousEventIndex = iEvent.Index();
#endif
    OnPointerEventImpl<false>(iContext, iEvent, &iArea, nullptr, iListener);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<bool PremaskMode>
void SensitiveArea::OnPointerEventImpl(PointerEventContext const& iContext, PointerEvent const& iEvent, IArea const* iArea, PremaskState const*iState, ISensitiveAreaListener& iListener)
{
    SG_CODE_FOR_ASSERT(SensitiveAreaListener_PleaseUseMacroForParameters_t const dummyParameter = SensitiveAreaListener_PleaseUseMacroForParameters;)
    SG_ASSERT((nullptr == iArea) == PremaskMode);
    SG_ASSERT((nullptr == iState) == !PremaskMode);
    system::UserInputEvent const& event = iEvent.Event();
    if(system::UserInputEventType::ResetAll == event.EventType())
    {
        Reset(iListener);
    }
    else if(system::UserInputDeviceType::Mouse == event.DeviceType())
    {
        if(system::UserInputEventType::ResetDevice == event.EventType())
        {
            Reset(iListener);
        }
        else if(system::UserInputEventType::ResetEntry == event.EventType())
        {
            SG_ASSERT_NOT_IMPLEMENTED();
        }
        else
        {
            system::MouseState const mouseState(event.State());
            if(m_buttonsMessedUp)
            {
                SG_ASSERT(m_buttonDownFromInside.none());
                SG_ASSERT(m_buttonDownFromOutside.none());
                SG_ASSERT(all_ones == m_buttonDownIfOne);
                SG_ASSERT(!m_pointerInside);
                if(mouseState.NoMouseButtonDown())
                {
                    m_buttonsMessedUp = false;
                }
            }
            else if(mouseState.StrictlyMoreThanOneMouseButtonDown())
            {
                if(m_pointerInside || m_buttonDownFromInside.any())
                {
                    // NB: We don't call Reset here, as we want to continue
                    // checking that all events are received, and
                    // m_buttonsMessedUp mut be set to true.
                    m_pointerInside = false;
                    m_buttonDownFromInside.reset();
                    m_buttonDownFromOutside.reset();
                    m_buttonDownIfOne = all_ones;
                    iListener.OnReset(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this);
                }
                else
                {
                    SG_ASSERT(m_buttonDownFromInside.none());
                    SG_ASSERT(m_buttonDownFromOutside.none());
                    SG_ASSERT(all_ones == m_buttonDownIfOne);
                    SG_ASSERT(!m_pointerInside);
                }
                m_buttonsMessedUp = true;
            }

            if(!m_buttonsMessedUp)
            {
                bool const isMasked = PremaskMode ? !event.CanBePreMasked() : event.IsMasked();
                bool const needToTreatOutsideEvent = m_pointerInside || m_buttonDownFromInside.any();
                bool const mayNeedToTreatEvent = !isMasked || needToTreatOutsideEvent;
                float3 localPointerPosition = PremaskMode ? iState->pointerLocalPosition : float3(0);
                bool const isInsideArea = mayNeedToTreatEvent ?
                    PremaskMode ? iState->isInsideArea : IsInside_AssumePointerEvent(iContext, event, *iArea, localPointerPosition) :
                    false;
                if(needToTreatOutsideEvent || isInsideArea)
                {
                    //float3 localPointerPosition(0);
                    //bool const isInsideArea = IsInside_AssumePointerEvent(iContext, event, iArea, localPointerPosition);
                    bool const isInsideNotMasked = !isMasked && isInsideArea;
                    if(PremaskMode)
                    {
                        if(iState->eventIsPremasked && event.IsPreMasked())
                            event.SetMaskedFromPremasked();
                        SG_ASSERT(event.IsMasked() || !isInsideArea);
                    }
                    else
                    {
                        if(isInsideNotMasked)
                            event.SetMasked();
                    }
                    // NB: We treat enter/leave event not only on Position
                    // entry as a previous event may have changed the
                    // position of the sensible area, or have created a
                    // new component in front of it, masking the event...
                    if(isInsideNotMasked)
                    {
                        if(m_pointerInside)
                        {
                            if(static_cast<i32>(system::MouseEntry::Position) == event.EntryId())
                            {
                                if(m_buttonDownFromInside.any())
                                {
                                    SG_ASSERT(m_buttonDownFromOutside.none());
                                    size_t const button = m_buttonDownIfOne;
                                    SG_ASSERT(all_ones != button);
                                    SG_ASSERT(1 == m_buttonDownFromInside.count());
                                    SG_ASSERT(m_buttonDownFromInside[button]);
                                    iListener.OnPointerMoveButtonDownFromInside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                                }
                                else if(m_buttonDownFromOutside.any())
                                {
                                    SG_ASSERT(m_buttonDownFromInside.none());
                                    size_t const button = m_buttonDownIfOne;
                                    SG_ASSERT(all_ones != button);
                                    SG_ASSERT(1 == m_buttonDownFromOutside.count());
                                    SG_ASSERT(m_buttonDownFromOutside[button]);
                                    iListener.OnPointerMoveButtonDownFromOutside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                                }
                                else
                                {
                                    SG_ASSERT(all_ones == m_buttonDownIfOne);
                                    iListener.OnPointerMoveFree(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition);
                                }
                            }
                        }
                        else
                        {
                            if(m_buttonDownFromInside.any())
                            {
                                SG_ASSERT(m_buttonDownFromOutside.none());
                                size_t const button = m_buttonDownIfOne;
                                SG_ASSERT(all_ones != button);
                                SG_ASSERT(1 == m_buttonDownFromInside.count());
                                SG_ASSERT(m_buttonDownFromInside[button]);
                                iListener.OnPointerEnterButtonDownFromInside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                            }
                            else if(mouseState.AnyMouseButtonDown())
                            {
                                SG_ASSERT(m_buttonDownFromInside.none());
                                SG_ASSERT(m_buttonDownFromOutside.none());
                                SG_ASSERT(!mouseState.StrictlyMoreThanOneMouseButtonDown());
                                u8 const mouseButtonsDown = mouseState.GetMouseButtonsDown();
                                size_t button = all_ones;
                                switch(mouseButtonsDown)
                                {
                                case static_cast<i32>(system::MouseEntry::Button0) : button = 0; break;
                                case static_cast<i32>(system::MouseEntry::Button1) : button = 1; break;
                                case static_cast<i32>(system::MouseEntry::Button2) : button = 2; break;
                                case static_cast<i32>(system::MouseEntry::Button3) : button = 3; break;
                                case static_cast<i32>(system::MouseEntry::Button4) : button = 4; break;
                                default:
                                    SG_ASSUME_NOT_REACHED();
                                }
                                SG_ASSERT(all_ones != button);
                                SG_ASSERT(button < BUTTON_COUNT);
                                m_buttonDownIfOne = checked_numcastable(button);
                                m_buttonDownFromOutside.set(button);
                                iListener.OnPointerEnterButtonDownFromOutside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                            }
                            else
                            {
                                SG_ASSERT(m_buttonDownFromInside.none());
                                SG_ASSERT(m_buttonDownFromOutside.none());
                                SG_ASSERT(all_ones == m_buttonDownIfOne);
                                iListener.OnPointerEnterFree(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition);
                            }
                        }
                    }
                    else
                    {
                        if(m_pointerInside)
                        {
                            if(m_buttonDownFromInside.any())
                            {
                                SG_ASSERT(m_buttonDownFromOutside.none());
                                size_t const button = m_buttonDownIfOne;
                                SG_ASSERT(all_ones != button);
                                SG_ASSERT(1 == m_buttonDownFromInside.count());
                                SG_ASSERT(m_buttonDownFromInside[button]);
                                iListener.OnPointerLeaveButtonDownFromInside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                            }
                            else if(m_buttonDownFromOutside.any())
                            {
                                SG_ASSERT(m_buttonDownFromInside.none());
                                size_t const button = m_buttonDownIfOne;
                                SG_ASSERT(all_ones != button);
                                SG_ASSERT(1 == m_buttonDownFromOutside.count());
                                SG_ASSERT(m_buttonDownFromOutside[button]);
                                iListener.OnPointerLeaveButtonDownFromOutside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                                m_buttonDownFromOutside.reset();
                                m_buttonDownIfOne = all_ones;
                            }
                            else
                            {
                                SG_ASSERT(m_buttonDownFromInside.none());
                                SG_ASSERT(m_buttonDownFromOutside.none());
                                SG_ASSERT(all_ones == m_buttonDownIfOne);
                                iListener.OnPointerLeaveFree(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition);
                            }
                        }
                        else
                        {
                            SG_ASSERT(m_buttonDownFromInside.any());
                            SG_ASSERT(m_buttonDownFromOutside.none());
                            size_t const button = m_buttonDownIfOne;
                            SG_ASSERT(all_ones != button);
                            SG_ASSERT(1 == m_buttonDownFromInside.count());
                            SG_ASSERT(m_buttonDownFromInside[button]);
                            iListener.OnPointerMoveOutsideButtonDownFromInside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, button);
                        }
                    }

                    size_t buttonId = all_ones;
                    switch(event.EntryId())
                    {
                    case static_cast<i32>(system::MouseEntry::Button0) : buttonId = 0; break;
                    case static_cast<i32>(system::MouseEntry::Button1) : buttonId = 1; break;
                    case static_cast<i32>(system::MouseEntry::Button2) : buttonId = 2; break;
                    case static_cast<i32>(system::MouseEntry::Button3) : buttonId = 3; break;
                    case static_cast<i32>(system::MouseEntry::Button4) : buttonId = 4; break;
                    case static_cast<i32>(system::MouseEntry::Position) : /* Do nothing */ break;
                    case static_cast<i32>(system::MouseEntry::Wheel) : /* Do nothing */ break;
                    default: break;
                    }

                    if(all_ones != buttonId)
                    {
                        switch(event.EventType())
                        {
                        case system::UserInputEventType::OffToOn:
                            SG_ASSERT(m_buttonDownFromInside.none());
                            SG_ASSERT(m_buttonDownFromOutside.none());
                            SG_ASSERT(all_ones == m_buttonDownIfOne);
                            iListener.OnButtonUpToDown(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, buttonId);
                            m_buttonDownFromInside.set(buttonId);
                            m_buttonDownIfOne = checked_numcastable(buttonId);
                            break;
                        case system::UserInputEventType::OnRepeat:
                            SG_ASSUME_NOT_REACHED();
                            break;
                        case system::UserInputEventType::OnToOff:
                            if(m_buttonDownFromInside.any())
                            {
                                SG_ASSERT(m_buttonDownFromOutside.none());
                                SG_ASSERT(buttonId == m_buttonDownIfOne);
                                SG_ASSERT(m_buttonDownFromInside[buttonId]);
                                if(m_pointerInside)
                                    iListener.OnButtonDownToUp(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, buttonId);
                                else
                                    iListener.OnButtonDownToUpOutside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, buttonId);
                                m_buttonDownFromInside.reset(buttonId);
                                m_buttonDownIfOne = all_ones;
                                SG_ASSERT(m_buttonDownFromInside.none());
                            }
                            else
                            {
                                SG_ASSERT(m_buttonDownFromInside.none());
                                SG_ASSERT(!isInsideNotMasked || m_buttonDownFromOutside.any());
                                SG_ASSERT(!isInsideNotMasked || buttonId == m_buttonDownIfOne);
                                SG_ASSERT(!isInsideNotMasked || m_buttonDownFromOutside[buttonId]);
                                iListener.OnButtonDownToUpFromOutside(SG_CODE_FOR_ASSERT(dummyParameter SG_COMMA) this, iContext, iEvent, localPointerPosition, buttonId);
                                m_buttonDownFromOutside.reset(buttonId);
                                m_buttonDownIfOne = all_ones;
                                SG_ASSERT(m_buttonDownFromOutside.none());
                            }
                            break;
                        case system::UserInputEventType::ResetEntry:
                            SG_ASSUME_NOT_REACHED();
                            break;
                        default:
                            SG_ASSUME_NOT_REACHED();
                        }
                    }
                    m_pointerInside = isInsideNotMasked;
                }
                else
                {
                    SG_ASSERT(!m_pointerInside);
                }
            }
        }
    }
}
//=============================================================================
}
}
