#ifndef System_UserInputEventAdapter_Win_H
#define System_UserInputEventAdapter_Win_H

#include <Core/Platform.h>
#if !SG_PLATFORM_IS_WIN
#error "Windows only !"
#endif

#include <Core/Assert.h>
#include <Math/Vector.h>
#include "MouseUtils.h"

namespace sg {
namespace system {
    class UserInputListenerList;
    class UserInputManager;
    class Window;
}
}

namespace sg {
namespace system {
namespace win {
//=============================================================================
class MouseAdapter
{
public:
    MouseAdapter();
    LRESULT WndProc(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
private:
    LRESULT OnReset(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
    LRESULT OnMove(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
    LRESULT OnButtonDown(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, MouseEntry button, UserInputListenerList& iWndListeners, UserInputManager& iManager);
    LRESULT OnButtonUp(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, MouseEntry button, UserInputListenerList& iWndListeners, UserInputManager& iManager);
    LRESULT OnMouseWheel(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
private:
    LRESULT OnMove(Window* iWnd, i16vec2 pos, u8 buttonsDown, UserInputListenerList& iWndListeners, UserInputManager& iManager);
    void CallTrackMouseEventIFN(Window* iWnd) { if(iWnd != m_mouseTracked && iWnd != m_mouseCaptured) { CallTrackMouseEvent(iWnd); } }
    void CallTrackMouseEvent(Window* iWnd);
    void CallSetCaptureIFN(Window* iWnd) { if(iWnd != m_mouseCaptured) { CallSetCapture(iWnd); } }
    void CallSetCapture(Window* iWnd);
    void CallReleaseCaptureIFN(Window* iWnd) { if(iWnd == m_mouseCaptured) { CallReleaseCapture(iWnd); } }
    void CallReleaseCapture(Window* iWnd);
private:
    i16vec2 m_pos;
    Window* m_mouseTracked;
    Window* m_mouseCaptured;
    SG_CODE_FOR_ASSERT(u8 m_buttonsDownForDebug);
};
//=============================================================================
class KeyboardAdapter
{
public:
    LRESULT WndProc(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
private:
    LRESULT OnKeyDown(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
    LRESULT OnKeyUp(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
};
//=============================================================================
class UserInputAdapter
{
public:
    LRESULT WndProc(Window* iWnd, UINT message, WPARAM wParam, LPARAM lParam, UserInputListenerList& iWndListeners, UserInputManager& iManager);
private:
    MouseAdapter m_mouseAdapter;
    KeyboardAdapter m_keyboardAdapter;
};
//=============================================================================
}
}
}

#endif
