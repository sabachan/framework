#ifndef System_UserInputEvent_H
#define System_UserInputEvent_H

#include <Core/Assert.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>

// An event should be masked by a client as soon as it is used to perform an
// action to prevent other client to interpret it. However, other clients also
// receive the event in order to reset their state if needed.
// An event should be masked when a client is an obstacle to event (eg. a gui
// window should mask any mouse event as soon as pointer is in window area).
//
// Premasking is used to enable nested clients to interpret event with a
// priority to nested clients, when an opaque client is present at the bottom.
// The opaque client is then informed that, as event will be masked by another,
// it should not be masked so that an outer client can interpret it (One
// exemple is for interpreting mouse wheel event when several scrolling
// containers are nested).
//
// additional data is used to add data to entry event (value for mouse wheel,
// is double click for mouse button, ...).
//
// state is used to add the device's state as a context (mouse position for
// mouse, standard modifiers key for mouse or keyboard, josticks positions for
// pads, ...). This value should not be interpreted independently of entry. If
// one needs to interpret and mask such value, it must be done on event
// associated with that value.
// Such state is representative of the state before the transition the event
// represents.
//
// Here are the event type meanings:
// - OffToOn, OnRepeat and OnToOff are used to represent transitions of binary
//   inputs, such as buttons, keys, etc.
// - Value is used to transmit the value of an analog input which has an
//   absolute, as joystick, mouse position, etc.
// - Message is used to transmit a one time event. It can be a text entry, the
//   print screen command, a mouse wheel update or any other message.
// - ResetEntry, ResetDevice and ResetAll are used to reset the corresponding
//   system to its default state but forbidding any client to activate a
//   feature on the corresponding transitions.

namespace sg {
namespace system {
//=============================================================================
class Window;
//=============================================================================
enum class UserInputDeviceType : u8 { None, Keyboard, Mouse, Pad, Touch, System, };
//=============================================================================
enum class UserInputEventType : u8 { None, OffToOn, OnRepeat, OnToOff, Value, Message, ResetEntry, ResetDevice, ResetAll };
//=============================================================================
SG_DEFINE_TYPED_TAG(reset_all)
//=============================================================================
class UserInputEvent : public SafeCountable
{
    SG_NON_COPYABLE(UserInputEvent)
    friend class UserInputEventClipper;
public:
    UserInputEvent(reset_all_t);
#if SG_ENABLE_ASSERT
    UserInputEvent(Window const* iWindow, size_t iIndex, size_t iBatchIndex);
#else
    UserInputEvent(Window const* iWindow);
#endif
    ~UserInputEvent();
    void SetMasked() const { SG_ASSERT(!m_masked && !m_premasked); m_masked = true; }
    void SetMaskedFromPremasked() const { SG_ASSERT(!m_masked && m_premasked); m_masked = true; m_premasked = false; }
    void SetPremasked() const { SG_ASSERT(!m_masked); m_premasked = true; }
    void SetDevice(UserInputDeviceType iDeviceType, u8 iDeviceId);
    void SetEntry(UserInputEventType iEventType, u32 iEntryId);
    void SetAdditionalData(u32 iData);
    void SetState(u64 iState);

    bool IsMasked() const { return m_masked || m_premasked; }
    bool CanBePreMasked() const { return !m_masked; }
    bool IsPreMasked() const { return m_premasked; }
    UserInputDeviceType DeviceType() const { return m_deviceType; }
    u8 DeviceId() const { return m_deviceId; }
    UserInputEventType EventType() const { return m_type; }
    u32 EntryId() const { return m_entryId; }
    u32 AdditionalData() const { return m_additionalData; }
    u64 State() const { return m_state; }
    Window const* WinIFP() const { return m_window.get(); }
#if SG_ENABLE_ASSERT
    size_t BatchIndex() const { return m_batchIndex; }
    size_t Index() const { return m_index; }
#endif
private:
    mutable bool m_masked;
    mutable bool m_premasked;
    UserInputDeviceType m_deviceType;
    u8 m_deviceId;
    UserInputEventType m_type;
    u32 m_entryId;
    u32 m_additionalData;
    u64 m_state;
    safeptr<Window const> m_window;
#if SG_ENABLE_ASSERT
    size_t m_batchIndex;
    size_t m_index;
#endif
};
//=============================================================================
// This class is used to clip an event for a sub part of the listeners graph.
// It should be instanciated in the stack, so that clipping is removed after
// exiting the function.
class UserInputEventClipper
{
    SG_NON_COPYABLE(UserInputEventClipper)
    SG_NON_NEWABLE
public:
    UserInputEventClipper(UserInputEvent const& iEvent, bool iClip)
        : m_event(&iEvent)
        , m_clipped(false)
    {
        if(!iEvent.m_masked && iClip)
        {
            m_clipped = true;
            iEvent.m_masked = true;
        }
    }
    ~UserInputEventClipper()
    {
        if(m_clipped)
            m_event->m_masked = false;
    }
private:
    safeptr<UserInputEvent const> m_event;
    bool m_clipped;
};
//=============================================================================
}
}

#endif
