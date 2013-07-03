#include "stdafx.h"

#include "UserInputAdapter_Win.h"
#include "UserInputListener.h"
#include "UserInputManager.h"
#include "MouseUtils.h"
#include "Window.h"
#include <Core/Cast.h>
#include <Core/Log.h>
#include <Core/StringFormat.h>
#include <Windowsx.h>

namespace sg {
namespace system {
namespace win {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u8 const MOUSE_BUTTONS_MASK = static_cast<u8>(MouseEntry::Button0)
                            | static_cast<u8>(MouseEntry::Button1)
                            | static_cast<u8>(MouseEntry::Button2)
                            | static_cast<u8>(MouseEntry::Button3)
                            | static_cast<u8>(MouseEntry::Button4);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u8 TranslateMouseButtonDown(WPARAM wParam)
{
    u8 const buttonsDown =
        ((MK_LBUTTON | MK_RBUTTON) & wParam)
        | (((MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2) & wParam) >> 2)
        | (((MK_CONTROL | MK_SHIFT) & wParam) << 3);
#if SG_ENABLE_ASSERT
    u16 refButtonsDown = 0;
    if(MK_LBUTTON  & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Button0); }
    if(MK_RBUTTON  & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Button1); }
    if(MK_MBUTTON  & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Button2); }
    if(MK_XBUTTON1 & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Button3); }
    if(MK_XBUTTON2 & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Button4); }
    if(MK_SHIFT    & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Shift); }
    if(MK_CONTROL  & wParam) { refButtonsDown |= static_cast<u16>(MouseEntry::Control); }
    SG_ASSERT(buttonsDown == refButtonsDown);
#endif
    return buttonsDown;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
MouseAdapter::MouseAdapter()
    : m_pos(0)
    , m_mouseTracked(nullptr)
    , m_mouseCaptured(nullptr)
    SG_CODE_FOR_ASSERT(SG_COMMA m_buttonsDownForDebug(0))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::OnReset(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED((wParam, lParam));
    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Mouse, 0);
    event.SetEntry(UserInputEventType::ResetDevice, 0);
    event.SetAdditionalData(0);
    event.SetState(0);

    m_pos = i16vec2(0);
    SG_CODE_FOR_ASSERT(m_buttonsDownForDebug = 0);

    switch(message)
    {
    case WM_CAPTURECHANGED:
        SG_ASSERT(nullptr == m_mouseTracked);
        // Following assert is deactivated as it seems that we receive this
        // message when clicking on window borders, even if we did'nt have
        // the capture.
        // SG_ASSERT(iWnd == m_mouseCaptured);
        SG_ASSERT(iWnd == m_mouseCaptured || nullptr == m_mouseCaptured);
        m_mouseCaptured = nullptr;
        break;
    case WM_MOUSELEAVE:
        SG_ASSERT(iWnd == m_mouseTracked);
        SG_ASSERT(nullptr == m_mouseCaptured);
        m_mouseTracked = nullptr;
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void MouseAdapter::CallTrackMouseEvent(Window* iWnd)
{
    SG_ASSERT(nullptr != iWnd);
    SG_ASSERT(iWnd != m_mouseTracked);
    SG_ASSERT(nullptr == m_mouseCaptured);
    TRACKMOUSEEVENT param;
    param.cbSize = sizeof(param);
    param.dwFlags = TME_LEAVE;
    param.hwndTrack = iWnd->hWnd();
    param.dwHoverTime = HOVER_DEFAULT;
    BOOL r = TrackMouseEvent(&param);
    SG_ASSERT_AND_UNUSED(r);
    m_mouseTracked = iWnd;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void MouseAdapter::CallSetCapture(Window* iWnd)
{
    SG_ASSERT(nullptr != iWnd);
    SG_ASSERT(iWnd == m_mouseTracked);
    SG_ASSERT(nullptr == m_mouseCaptured);
    TRACKMOUSEEVENT param;
    param.cbSize = sizeof(param);
    param.dwFlags = TME_CANCEL | TME_LEAVE;
    param.hwndTrack = iWnd->hWnd();
    param.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&param);
    m_mouseTracked = nullptr;
#if SG_ENABLE_ASSERT
    param.cbSize = sizeof(param);
    param.dwFlags = TME_QUERY;
    param.hwndTrack = 0;
    param.dwHoverTime = 0;
    TrackMouseEvent(&param);
    SG_ASSERT(0 == param.dwFlags);
    SG_ASSERT(iWnd->hWnd() == param.hwndTrack || nullptr == param.hwndTrack);
#endif
    SetCapture(iWnd->hWnd());
    m_mouseCaptured = iWnd;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void MouseAdapter::CallReleaseCapture(Window* iWnd)
{
    SG_ASSERT(nullptr != iWnd);
    SG_ASSERT(nullptr == m_mouseTracked);
    SG_ASSERT(iWnd == m_mouseCaptured);
    BOOL const r = ReleaseCapture();
    SG_ASSERT_AND_UNUSED(r);
    m_mouseCaptured = nullptr;
    CallTrackMouseEvent(iWnd);
    SG_ASSERT(iWnd == m_mouseTracked);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::OnMove(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED(message);
#if SG_ENABLE_ASSERT
    if(nullptr != m_mouseTracked)
    {
        SG_ASSERT(nullptr != iWnd);
        SG_ASSERT(iWnd == m_mouseTracked);
        TRACKMOUSEEVENT param;
        param.cbSize = sizeof(param);
        param.dwFlags = TME_QUERY;
        param.hwndTrack = 0;
        param.dwHoverTime = 0;
        BOOL const r = TrackMouseEvent(&param);
        SG_ASSERT(r);
        SG_ASSERT(TME_LEAVE == param.dwFlags);
        SG_ASSERT(iWnd->hWnd() == param.hwndTrack);
    }
    else if(nullptr != m_mouseCaptured)
    {
        SG_ASSERT(iWnd == m_mouseCaptured);
        SG_ASSERT(iWnd->hWnd() == GetCapture());
    }
#endif

    i16 const xPos = checked_numcastable(GET_X_LPARAM(lParam));
    i16 const yPos = checked_numcastable(GET_Y_LPARAM(lParam));
    i16vec2 const pos(xPos, yPos);
    u8 const buttonsDown = TranslateMouseButtonDown(wParam);

    return OnMove(iWnd, pos, buttonsDown, iWndListeners, iManager);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::OnMove(Window* iWnd, i16vec2 pos, u8 buttonsDown, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_ASSERT((m_buttonsDownForDebug & MOUSE_BUTTONS_MASK) == (buttonsDown & MOUSE_BUTTONS_MASK) || nullptr == m_mouseTracked);

    u32 const entryId = checked_numcastable(MouseEntry::Position);
    u32 const additionalData = 0;
    MouseState mouseState;
    mouseState.SetPosition(pos);
    mouseState.SetButtonsDown(buttonsDown);

    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Mouse, 0);
    event.SetEntry(UserInputEventType::Value, entryId);
    event.SetAdditionalData(additionalData);
    event.SetState(mouseState.Data());

    m_pos = pos;

    CallTrackMouseEventIFN(iWnd);

    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::OnButtonDown(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, MouseEntry button, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED(message);
    i16 const xPos = checked_numcastable(GET_X_LPARAM(lParam));
    i16 const yPos = checked_numcastable(GET_Y_LPARAM(lParam));
    i16vec2 const pos(xPos, yPos);

    u8 const buttonsDown = TranslateMouseButtonDown(wParam);
    u8 const prevButtonsDown = buttonsDown & ~static_cast<u8>(button);
    SG_ASSERT(((m_buttonsDownForDebug & MOUSE_BUTTONS_MASK) == (prevButtonsDown & MOUSE_BUTTONS_MASK)) || nullptr == m_mouseTracked);

    if(pos != m_pos)
        OnMove(iWnd, pos, prevButtonsDown, iWndListeners, iManager);
    SG_ASSERT(pos == m_pos);

    SG_ASSERT(MouseEntry::Button0 == button
           || MouseEntry::Button1 == button
           || MouseEntry::Button2 == button
           || MouseEntry::Button3 == button
           || MouseEntry::Button4 == button);
    u32 const entryId = checked_numcastable(button);
    MouseButtonAdditionalData additionalData;
    //additionalData.SetDoubleClick(false/true);
    MouseState mouseState;
    mouseState.SetPosition(pos);
    mouseState.SetButtonsDown(prevButtonsDown);

    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Mouse, 0);
    event.SetEntry(UserInputEventType::OffToOn, entryId);
    event.SetAdditionalData(additionalData.Data());
    event.SetState(mouseState.Data());

    SG_CODE_FOR_ASSERT(m_buttonsDownForDebug = buttonsDown);

    CallSetCaptureIFN(iWnd);

    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::OnButtonUp(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, MouseEntry button, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED(message);
    i16 const xPos = checked_numcastable(GET_X_LPARAM(lParam));
    i16 const yPos = checked_numcastable(GET_Y_LPARAM(lParam));
    i16vec2 const pos(xPos, yPos);

    u8 const buttonsDown = TranslateMouseButtonDown(wParam);
    u8 const prevButtonsDown = buttonsDown | static_cast<u8>(button);
    SG_ASSERT(((m_buttonsDownForDebug & MOUSE_BUTTONS_MASK) == (prevButtonsDown & MOUSE_BUTTONS_MASK)) || nullptr == m_mouseTracked);

    if(pos != m_pos)
        OnMove(iWnd, pos, prevButtonsDown, iWndListeners, iManager);
    SG_ASSERT(pos == m_pos);

    SG_ASSERT(MouseEntry::Button0 == button
           || MouseEntry::Button1 == button
           || MouseEntry::Button2 == button
           || MouseEntry::Button3 == button
           || MouseEntry::Button4 == button);
    u32 const entryId = checked_numcastable(button);
    MouseButtonAdditionalData additionalData;
    //additionalData.SetDoubleClick(false/true);
    MouseState mouseState;
    mouseState.SetPosition(pos);
    mouseState.SetButtonsDown(prevButtonsDown);

    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Mouse, 0);
    event.SetEntry(UserInputEventType::OnToOff, entryId);
    event.SetAdditionalData(additionalData.Data());
    event.SetState(mouseState.Data());

    SG_CODE_FOR_ASSERT(m_buttonsDownForDebug = buttonsDown);

    if(0 == buttonsDown)
        CallReleaseCaptureIFN(iWnd);

    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::OnMouseWheel(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED(message);
    i16 const xPos = checked_numcastable(GET_X_LPARAM(lParam));
    i16 const yPos = checked_numcastable(GET_Y_LPARAM(lParam));
    i16vec2 const posInScreen(xPos, yPos);
    i16vec2 const pos = posInScreen - i16vec2(iWnd->GetClientTopLeft());

    u16 const keys = GET_KEYSTATE_WPARAM(wParam);
    i16 const zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    SG_ASSERT(std::abs(zDelta) < (128 << 16));
    i32 const wheelDeltaFP16 = ((zDelta << 16) / WHEEL_DELTA);
    u8 const buttonsDown = TranslateMouseButtonDown(keys);
    u8 const prevButtonsDown = buttonsDown;

    if(pos != m_pos)
        OnMove(iWnd, pos, prevButtonsDown, iWndListeners, iManager);
    SG_ASSERT(pos == m_pos);

    u32 const entryId = checked_numcastable(MouseEntry::Wheel);
    MouseWheelAdditionalData additionalData;
    additionalData.SetWheelDeltaFP16(wheelDeltaFP16);
    MouseState mouseState;
    mouseState.SetPosition(pos);
    mouseState.SetButtonsDown(buttonsDown);

    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Mouse, 0);
    event.SetEntry(UserInputEventType::Message, entryId);
    event.SetAdditionalData(additionalData.Data());
    event.SetState(mouseState.Data());

    SG_CODE_FOR_ASSERT(m_buttonsDownForDebug = buttonsDown);

    if(0 == buttonsDown)
        CallReleaseCaptureIFN(iWnd);

    return 0;

}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT MouseAdapter::WndProc(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_ASSERT(nullptr == m_mouseTracked || nullptr == m_mouseCaptured);
    SG_ASSERT(nullptr == m_mouseTracked || iWnd == m_mouseTracked);
    SG_ASSERT(nullptr == m_mouseCaptured || iWnd == m_mouseCaptured);
    SG_ASSERT(WM_MOUSEFIRST <= message && message <= WM_MOUSELAST
           || WM_MOUSEHOVER == message || WM_MOUSELEAVE == message
           || WM_CAPTURECHANGED == message);
    switch (message)
    {
    case WM_CAPTURECHANGED:
        return OnReset(iWnd, message, wParam, lParam, iWndListeners, iManager);
    case WM_MOUSELEAVE:
        return OnReset(iWnd, message, wParam, lParam, iWndListeners, iManager);
    case WM_MOUSEHOVER:
        SG_ASSERT_NOT_REACHED(); // not requested for now
        break;

    case WM_MOUSEMOVE:
        return OnMove(iWnd, message, wParam, lParam, iWndListeners, iManager);
    case WM_LBUTTONDOWN:
        return OnButtonDown(iWnd, message, wParam, lParam, MouseEntry::Button0, iWndListeners, iManager);
    case WM_LBUTTONUP:
        return OnButtonUp(iWnd, message, wParam, lParam, MouseEntry::Button0, iWndListeners, iManager);
    case WM_LBUTTONDBLCLK:
        break;
    case WM_RBUTTONDOWN:
        return OnButtonDown(iWnd, message, wParam, lParam, MouseEntry::Button1, iWndListeners, iManager);
        break;
    case WM_RBUTTONUP:
        return OnButtonUp(iWnd, message, wParam, lParam, MouseEntry::Button1, iWndListeners, iManager);
        break;
    case WM_RBUTTONDBLCLK:
        break;
    case WM_MBUTTONDOWN:
        return OnButtonDown(iWnd, message, wParam, lParam, MouseEntry::Button2, iWndListeners, iManager);
        break;
    case WM_MBUTTONUP:
        return OnButtonUp(iWnd, message, wParam, lParam, MouseEntry::Button2, iWndListeners, iManager);
        break;
    case WM_MBUTTONDBLCLK:
        break;
    case WM_MOUSEWHEEL:
        return OnMouseWheel(iWnd, message, wParam, lParam, iWndListeners, iManager);
        break;
    case WM_XBUTTONDOWN:
        break;
    case WM_XBUTTONUP:
        break;
    case WM_XBUTTONDBLCLK:
        break;
    case WM_MOUSEHWHEEL:
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return 0;
}
//=============================================================================
LRESULT KeyboardAdapter::OnKeyDown(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED(message);
    u32 const vkCode = checked_numcastable(wParam);
    u8 const scanCode = (lParam >> 16) & 0xFF;
    u16 const repeatCount = lParam & 0xFFFF;
    SG_UNUSED(repeatCount);
    bool const isExtended = 1 == ((lParam >> 24) & 0x1);
    bool const isRepeat = 1 == ((lParam >> 30) & 0x1);

    u32 const entryId = scanCode | (isExtended << 8);
    u32 const additionalData = vkCode;
    u32 const keyboardState = 0; // TODO

    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Keyboard, 0);
    event.SetEntry(isRepeat ? UserInputEventType::OnRepeat : UserInputEventType::OffToOn, entryId);
    event.SetAdditionalData(additionalData);
    event.SetState(keyboardState);

    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT KeyboardAdapter::OnKeyUp(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_UNUSED(message);
    u32 const vkCode = checked_numcastable(wParam);
    u8 const scanCode = (lParam >> 16) & 0xFF;
    u16 const repeatCount = lParam & 0xFFFF;
    SG_UNUSED(repeatCount);
    bool const isExtended = 1 == ((lParam >> 24) & 0x1);
    bool const isRepeat = 1 == ((lParam >> 30) & 0x1);
    SG_UNUSED(isRepeat);

    u32 const entryId = scanCode | (isExtended << 8);
    u32 const keyboardState = 0; // TODO

    UserInputEvent& event = iManager.CreatePushAndGetEvent(iWnd, &iWndListeners);
    event.SetDevice(UserInputDeviceType::Keyboard, 0);
    event.SetEntry(UserInputEventType::OnToOff, entryId);
    event.SetAdditionalData(vkCode);
    event.SetState(keyboardState);
    return 0;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
LRESULT KeyboardAdapter::WndProc(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_ASSERT(WM_KEYFIRST <= message && message <= WM_KEYLAST);
    switch (message)
    {
    case WM_KEYDOWN:
        return OnKeyDown(iWnd, message, wParam, lParam, iWndListeners, iManager);
    case WM_KEYUP:
        return OnKeyUp(iWnd, message, wParam, lParam, iWndListeners, iManager);
    case WM_CHAR:
        break;
    case WM_DEADCHAR:
        break;
    case WM_SYSKEYDOWN:
        break;
    case WM_SYSKEYUP:
        break;
    case WM_SYSCHAR:
        break;
    case WM_SYSDEADCHAR:
        break;
    case WM_UNICHAR:
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return 0;
}
//=============================================================================
LRESULT UserInputAdapter::WndProc(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager)
{
    SG_ASSERT(nullptr != iWnd);
    if(WM_MOUSEFIRST <= message && message <= WM_MOUSELAST
       || WM_MOUSEHOVER == message || WM_MOUSELEAVE == message
       || WM_CAPTURECHANGED == message)
        return m_mouseAdapter.WndProc(iWnd, message, wParam, lParam, iWndListeners, iManager);
    if(WM_KEYFIRST <= message && message <= WM_KEYLAST)
        return m_keyboardAdapter.WndProc(iWnd, message, wParam, lParam, iWndListeners, iManager);
    return DefWindowProc(iWnd->hWnd(), message, wParam, lParam);
}
//=============================================================================
}
}
}
